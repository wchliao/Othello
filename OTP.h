#include"board.h"
#include<random>
#ifdef _WIN32
#include<chrono>
#endif
#include<cstring>
#include<string>
#include<sys/time.h>
#include<signal.h>
#include<cmath>

// parameters
const double UCB_c = 0.8 ;
const double r_d = 1.5 ;
const double sigma_e = 1.5 ;
const int simulateN = 100 ;
const int alarm_time = 9 ;

// global flags
bool stopflag = false ;

// global variables

static void sig_handler(int signo){
	stopflag = true ;
	return ;
}

constexpr char m_tolower(char c){
	return c+('A'<=c&&c<='Z')*('a'-'A');
}

constexpr unsigned my_hash(const char*s,unsigned long long int hv=0){
	return *s&&*s!=' '?my_hash(s+1,(hv*('a'+1)+m_tolower(*s))%0X3FFFFFFFU):hv;
}

struct history {
	unsigned long long black ;
	unsigned long long white ;
	int pass ;
} ;

struct grade {
	int win ;
	int lose ;
	int draw ;
	int simulateCount ;

	int sum1 ;
	int sum2 ;
} ;

struct node {
	// basic members
	int win ;
	int lose ;
	int draw ;
	unsigned long long pos ;
	double winrate ;
	double pot ;
	board B ;

	int simulateCount ;

	int childCount ;
	struct node *child ;
	struct node *next ;

	int sum1 ;
	int sum2 ;
	double sv ;
	double ml ;
	double mr ;
} ;

void destroyTree(struct node *root){
	if( root->childCount > 0 ){
		struct node *cur = root->child ;
		struct node *next = cur->next ;
		destroyTree(cur) ;
		for( int i = 1 ; i < root->childCount ; i++ ){
			cur = next ;
			next = cur->next ;
			destroyTree(cur) ;
		}
	}
	delete root ;
	return ;
}

template<class RIT>RIT random_choice(RIT st,RIT ed){
#ifdef _WIN32
	//std::random_device is deterministic with MinGW gcc 4.9.2 on Windows
	static std::mt19937 local_rand(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
#else
	static std::mt19937 local_rand(std::random_device{}());
#endif
	return st+std::uniform_int_distribution<int>(0,ed-st-1)(local_rand);

}

class OTP{

	board B;
	history H[128],*Hp;

	//initialize in do_init
	void do_init(){

		B = board() ;
		Hp = H ;

		struct sigaction act ;
		act.sa_handler = sig_handler ;
		sigemptyset(&act.sa_mask) ;
		sigaddset(&act.sa_mask, SIGALRM) ;
		act.sa_flags = 0 ;
		act.sa_flags |= SA_INTERRUPT ;

		if( sigaction( SIGALRM, &act, NULL ) < 0 )
			fprintf(stderr, "sigaction fail.\n") ;
	}

	//choose the best move in do_genmove
	int do_genmove(){
		int ans ;

		struct itimerval time ;
		time.it_interval.tv_usec = 0 ;
		time.it_interval.tv_sec = 0 ;
		time.it_value.tv_usec = 0 ;
		time.it_value.tv_sec = alarm_time ;
		if( setitimer( ITIMER_REAL, &time, NULL) < 0 )
			fprintf(stderr, "set timer fail.\n") ;

		stopflag = false ;

		ans = genmove() ;

		time.it_value.tv_sec = 0 ;
		if( setitimer( ITIMER_REAL, &time, NULL) < 0 )
			fprintf(stderr, "set timer fail.\n") ;
		stopflag = false ;
		return ans ;
	}

	//update board and history in do_play
	void do_play(int x,int y){
		if(Hp!=std::end(H)&&B.is_game_over()==0&&B.is_valid_move(x,y)){
			Hp->black = B.get_black() ;
			Hp->white = B.get_white() ;
			Hp->pass = B.get_pass() ;
			++Hp ;
			B.update(x,y) ;
		}else{
			fputs("wrong play.\n",stderr);
			fprintf(stderr,"(x,y) = (%d,%d)\n",x,y);
		}
	}

	//undo board and history in do_undo
	void do_undo(){
		if(Hp!=H){
			--Hp;
			B.undo(Hp->black, Hp->white, Hp->pass);
		}else{
			fputs("wrong undo.\n",stderr);
		}
	}

	// find and print all valid move
	int* do_validmove(int *p){
		unsigned long long llp[64], *llpED(llp) ;
		llpED = B.get_valid_move(llp) ;
		if( llp == llpED )
			return p ;
		else {
			do{
				--llpED ;
				*p = __builtin_ctzll(*llpED) ;
				++p ;
			}while( llp != llpED ) ;
			return p ;
		}
	}

	int genmove(){
		if( no_valid_move() )
			return 64 ;

		bool my_tile = B.get_my_tile() ;
		struct node *root = new struct node ;
		root->win = 0 ;
		root->lose = 0 ;
		root->draw = 0 ;
		root->pos = 0 ;
		root->winrate = 0 ;
		root->pot = 0 ;
		root->simulateCount = 0 ;
		root->B = board(B.get_black(), B.get_white(), my_tile, B.get_pass()) ;
		root->childCount = 0 ;
		root->child = NULL ;
		root->next = NULL ;
		root->sum1 = 0 ;
		root->sum2 = 0 ;
		root->ml = 0 ;
		root->mr = 0 ;

		// Monte-Carlo
		struct grade g ;
		while(!stopflag){
			g = root_simulate(root, my_tile, my_tile) ;
			root->simulateCount += g.simulateCount ;
		}

		struct node *maxN = root->child ;
		struct node *tmpN = root->child->next ;
		for( int i = 1 ; i < root->childCount ; i++ ){
			if( tmpN->winrate > maxN->winrate )
				maxN = tmpN ;
			tmpN = tmpN->next ;
		}

		int pos = __builtin_ctzll(maxN->pos) ;
		destroyTree(root) ;
		return pos ;
	}

	struct grade root_simulate(struct node *root, bool my_tile, bool top_root_tile){
		struct grade g ;
		g.win = 0 ;
		g.lose = 0 ;
		g.draw = 0 ;
		g.simulateCount = 0 ;
		g.sum1 = 0 ;
		g.sum2 = 0 ;

		if( root->B.is_game_over() ){
			int score = root->B.get_score() ;
			if( top_root_tile ){
				if( score > 0 )
					g.lose = simulateN ;
				else if( score < 0 )
					g.win = simulateN ;
				else
					g.draw = simulateN ;
				g.sum1 = -simulateN*score ;
				g.sum2 = simulateN*score*score ;
			}
			else {
				if( score > 0 )
					g.win = simulateN ;
				else if( score < 0 )
					g.lose = simulateN;
				else
					g.draw = simulateN ;
				g.sum1 = simulateN*score ;
				g.sum2 = simulateN*score*score ;
			}
			g.simulateCount = simulateN ;
			root->simulateCount += g.simulateCount ; 
			return g ;
		}
		
		// Selection 
		struct node *tmpN ;
		if( root->childCount > 0 ){
			struct node *maxN = root->child ;
			tmpN = maxN->next ;
			for( int i = 1 ; i < root->childCount ; i++ ){
				if( tmpN->winrate + tmpN->pot > maxN->winrate + maxN->pot )
					maxN = tmpN ;
				tmpN = tmpN->next ;
			}
			g = root_simulate(maxN, !my_tile, top_root_tile) ;
			
			// Back propagation
			if( my_tile == top_root_tile ){
				maxN->win += g.win ;
				maxN->lose += g.lose ;
				maxN->draw += g.draw ;
				maxN->sum1 += g.sum1 ;
				maxN->sum2 += g.sum2 ;
			}
			else {
				maxN->win += g.lose ;
				maxN->lose += g.win ;
				maxN->draw += g.draw ;
				maxN->sum1 -= g.sum1 ;
				maxN->sum2 += g.sum2 ;
			}
			maxN->winrate = double(maxN->win)/maxN->simulateCount ;
			
			double mean = maxN->sum1/maxN->simulateCount ;
			maxN->sv = sqrt((maxN->sum2-2*mean*maxN->sum1)/maxN->simulateCount + mean*mean) ;
			maxN->ml = mean - r_d*maxN->sv ;
			maxN->mr = -(maxN->ml-2*mean) ;

			// Pruning
			bool maxcut = false ;
			double maxml, maxmlsv ;

			tmpN = root->child ;
			maxml = tmpN->ml ;
			maxmlsv = tmpN->sv ;
			tmpN = tmpN->next ;
			for( int i = 1 ; i < root->childCount ; i++ ){
				if( tmpN->ml > maxml ){
					maxml = tmpN->ml ;
					maxmlsv = tmpN->sv ;
				}
				tmpN = tmpN->next ;
			}

			struct node *prev ;
			struct node *cur = root->child ;
			int oldChildCount = root->childCount ;
			int oldsimulateCount = root->simulateCount ;
			root->simulateCount = 0 ;
			int i = 0 ;
			if( my_tile == top_root_tile ){
				do{
					if( maxmlsv < sigma_e && cur->sv < sigma_e && cur->mr < maxml ){
						if( maxN == cur )
							maxcut = true ;
						root->child = cur->next ;
						destroyTree(cur) ;
						--root->childCount ;
						cur = root->child ;
						++i ;
					}
					else {
						root->simulateCount += cur->simulateCount ;
						prev = root->child ;
						cur = cur->next ;
						++i ;
						break ;
					}
				}while( i < oldChildCount ) ;

				while( i < oldChildCount){
					if( maxmlsv < sigma_e && cur->sv < sigma_e && cur->mr < maxml ){
						if( maxN == cur )
							maxcut = true ;
						prev->next = cur->next ;
						destroyTree(cur) ;
						--root->childCount ;
						cur = prev->next ;
						++i ;
					}
					else {
						root->simulateCount += cur->simulateCount ;
						prev = cur ;
						cur = cur->next ;
						++i ;
					}
				}
			}
			else {
				do{
					if( maxmlsv < sigma_e && cur->sv < sigma_e && cur->mr < maxml ){
						if( maxN == cur )
							maxcut = true ;
						root->child = cur->next ;
						destroyTree(cur) ;
						--root->childCount ;
						cur = root->child ;
						++i ;
					}
					else {
						root->simulateCount += cur->simulateCount ;
						prev = root->child ;
						cur = cur->next ;
						++i ;
						break ;
					}
				}while( i < oldChildCount ) ;

				while( i < oldChildCount){
					if( maxmlsv < sigma_e && cur->sv < sigma_e && cur->mr < maxml ){
						if( maxN == cur )
							maxcut = true ;
						prev->next = cur->next ;
						destroyTree(cur) ;
						--root->childCount ;
						cur = prev->next ;
						++i ;
					}
					else {
						root->simulateCount += cur->simulateCount ;
						prev = cur ;
						cur = cur->next ;
						++i ;
					}
				}
			}
			
			double pot = sqrt(log(root->simulateCount)/log(oldsimulateCount)) ;
			tmpN = root->child ;
			for( int i = 0 ; i < root->childCount ; i++ ){
				if( tmpN != maxN )
					tmpN->pot *= pot ;
				tmpN = tmpN->next ;
			}
			if( maxcut ){
				g.win = 0 ;
				g.lose = 0 ;
				g.draw = 0 ;
				g.simulateCount = 0 ;
				g.sum1 = 0 ;
				g.sum2 = 0 ;
			}
			else{
				maxN->pot = UCB_c*sqrt(log(root->simulateCount)/maxN->simulateCount) ;
			}
		}
		else {
			unsigned long long ML[64], *MLED(root->B.get_valid_move(ML)) ;
			int nodeCount = MLED - ML ;

			// Expansion
			if( nodeCount == 0 ){
				nodeCount = 1 ;
				root->child = new struct node ;
				root->childCount = nodeCount ;
				root->child->pos = 0 ;
			}
			else {
				root->child = new struct node ;
				root->childCount = nodeCount ;
				tmpN = root->child ;
				for( int i = 1 ; i < nodeCount ; i++ ){
					tmpN->next = new struct node ;
					tmpN = tmpN->next ;
				}
				root->child->pos = ML[0] ;
			}

			double pot = UCB_c*sqrt(log(simulateN*nodeCount)/simulateN) ;
			root->child->win = 0 ;
			root->child->lose = 0 ;
			root->child->draw = 0 ;
			root->child->winrate = 0 ;
			root->child->pot = pot ;
			root->child->simulateCount = simulateN ;
			root->child->B = root->B ; ;
			root->child->childCount = 0 ;
			root->child->child = NULL ;
			root->child->B.update(root->child->pos) ;

			tmpN = root->child ;
			for( int i = 1 ; i < nodeCount ; i++ ){
				tmpN->next->win = 0 ;
				tmpN->next->lose = 0 ;
				tmpN->next->draw = 0 ;
				tmpN->next->winrate = 0 ;
				tmpN->next->pos = ML[i] ;
				tmpN->next->pot = pot ;
				tmpN->next->simulateCount = simulateN ;
				tmpN->next->B = root->B ;
				tmpN->next->childCount = 0 ;
				tmpN->next->child = NULL ;
				tmpN->next->B.update(tmpN->next->pos) ;
				tmpN = tmpN->next ;
			}
			tmpN->next = NULL ;

			// Simulations
			struct grade tmpg ;
			double maxml = -1000000 ;
			double maxmlsv = -1 ;
			double mean ;
			tmpN = root->child ;
			if( my_tile == top_root_tile ){
				for( int i = 0 ; i < nodeCount ; i++ ){
					tmpg = leaf_simulate(tmpN->B, top_root_tile) ;
					tmpN->win = tmpg.win ;
					tmpN->lose = tmpg.lose ;
					tmpN->draw = tmpg.draw ;
					tmpN->winrate = double(tmpN->win)/tmpN->simulateCount ;
					tmpN->sum1 = tmpg.sum1 ;
					tmpN->sum2 = tmpg.sum2 ;
					
					mean = tmpN->sum1/tmpN->simulateCount ;
					tmpN->sv = sqrt((tmpN->sum2-2*mean*tmpN->sum1)/tmpN->simulateCount + mean*mean) ;
					tmpN->ml = mean - r_d*tmpN->sv ;
					tmpN->mr = -(tmpN->ml-2*mean) ;
					if( tmpN->ml > maxml ){
						maxml = tmpN->ml ;
						maxmlsv = tmpN->sv ;
					}
					
					tmpN = tmpN->next ;
				}
			}
			else {
				for( int i = 0 ; i < nodeCount ; i++ ){
					tmpg = leaf_simulate(tmpN->B, top_root_tile) ;
					tmpN->win = tmpg.lose ;
					tmpN->lose = tmpg.win ;
					tmpN->draw = tmpg.draw ;
					tmpN->winrate = double(tmpN->win)/tmpN->simulateCount ;
					tmpN->sum1 = -tmpg.sum1 ;
					tmpN->sum2 = tmpg.sum2 ;
					
					mean = tmpN->sum1/tmpN->simulateCount ;
					tmpN->sv = sqrt((tmpN->sum2-2*mean*tmpN->sum1)/tmpN->simulateCount + mean*mean) ;
					tmpN->ml = mean - r_d*tmpN->sv ;
					tmpN->mr = -(tmpN->ml-2*mean) ;
					if( tmpN->ml > maxml ){
						maxml = tmpN->ml ;
						maxmlsv = tmpN->sv ;
					}
					
					tmpN = tmpN->next ;
				}
			}

			// Pruning
			struct node *prev ;
			struct node *cur = root->child ;
			int i = 0 ;
			if( my_tile == top_root_tile ){
				do{
					if( maxmlsv < sigma_e && cur->sv < sigma_e && cur->mr < maxml ){
						root->child = cur->next ;
						destroyTree(cur) ;
						--root->childCount ;
						cur = root->child ;
						++i ;
					}
					else {
						g.win += cur->win ;
						g.lose += cur->lose ;
						g.draw += cur->draw ;
						g.simulateCount += cur->simulateCount ;
						g.sum1 += cur->sum1 ;
						g.sum2 += cur->sum2 ;
						root->simulateCount += cur->simulateCount ;
						prev = root->child ;
						cur = cur->next ;
						++i ;
						break ;
					}
				}while( i < nodeCount ) ;

				while( i < nodeCount){
					if( maxmlsv < sigma_e && cur->sv < sigma_e && cur->mr < maxml ){
						prev->next = cur->next ;
						destroyTree(cur) ;
						--root->childCount ;
						cur = prev->next ;
						++i ;
					}
					else {
						g.win += cur->win ;
						g.lose += cur->lose ;
						g.draw += cur->draw ;
						g.simulateCount += cur->simulateCount ;
						g.sum1 += cur->sum1 ;
						g.sum2 += cur->sum2 ;
						root->simulateCount += cur->simulateCount ;
						prev = cur ;
						cur = cur->next ;
						++i ;
					}
				}
			}
			else {
				do{
					if( maxmlsv < sigma_e && cur->sv < sigma_e && cur->mr < maxml ){
						root->child = cur->next ;
						destroyTree(cur) ;
						--root->childCount ;
						cur = root->child ;
						++i ;
					}
					else {
						g.win += cur->lose ;
						g.lose += cur->win ;
						g.draw += cur->draw ;
						g.simulateCount += cur->simulateCount ;
						g.sum1 -= cur->sum1 ;
						g.sum2 += cur->sum2 ;
						root->simulateCount += cur->simulateCount ;
						prev = root->child ;
						cur = cur->next ;
						++i ;
						break ;
					}
				}while( i < nodeCount ) ;

				while( i < nodeCount){
					if( maxmlsv < sigma_e && cur->sv < sigma_e && cur->mr < maxml ){
						prev->next = cur->next ;
						destroyTree(cur) ;
						--root->childCount ;
						cur = prev->next ;
						++i ;
					}
					else {
						g.win += cur->lose ;
						g.lose += cur->win ;
						g.draw += cur->draw ;
						g.simulateCount += cur->simulateCount ;
						g.sum1 -= cur->sum1 ;
						g.sum2 += cur->sum2 ;
						root->simulateCount += cur->simulateCount ;
						prev = cur ;
						cur = cur->next ;
						++i ;
					}
				}
			}
		}

		// Back propagation
		return g ;
	}

	struct grade leaf_simulate(board B, bool my_tile){
		struct grade g ;
		unsigned long long ML[64], *MLED(ML) ;
		int score ;

		g.win = 0 ;
		g.lose = 0 ;
		g.draw = 0 ;
		g.sum1 = 0 ;
		g.sum2 = 0 ;

		for( int t = 0 ; t < simulateN ; t++ ){
			board tmpB = B ;
			while(!tmpB.is_game_over()){
				MLED = tmpB.get_valid_move(ML) ;
				if( ML == MLED )
					tmpB.update(0) ; // pass
				else 
					tmpB.update(*random_choice(ML,MLED)) ;
			}
			score = tmpB.get_score() ;
			if( my_tile ){
				if( score > 0 )
					++g.lose ;
				else if( score < 0 )
					++g.win ;
				else
					++g.draw ;

				g.sum1 -= score ;
				g.sum2 += score*score ;
			}
			else {
				if( score > 0 )
					++g.win ;
				else if( score < 0 )
					++g.lose ;
				else
					++g.draw ;

				g.sum1 += score ;
				g.sum2 += score*score ;
			}
		}
		g.simulateCount = simulateN ;
		return g ;
	}

	bool no_valid_move(){
		int ML[64], *MLED(B.get_valid_move(ML)) ;
		if( ML == MLED )
			return true ;
		else
			return false ;
	}

	// operations
	public:
	OTP():B(),Hp(H){
		do_init();
	}

	bool do_op(const char*cmd,char*out,FILE*myerr){
		switch(my_hash(cmd)){
			case my_hash("name"):
				sprintf(out,"name B02902105");
				return true;
			case my_hash("clear_board"):
				do_init();
				B.show_board(myerr);
				sprintf(out,"clear_board");
				return true;
			case my_hash("showboard"):
				B.show_board(myerr);
				sprintf(out,"showboard");
				return true;
			case my_hash("play"):{
				int x,y;
				sscanf(cmd,"%*s %d %d",&x,&y);
				do_play(x,y);
				B.show_board(myerr);
				sprintf(out,"play");
				fprintf(myerr,"\n") ;
				return true;
			}
			case my_hash("genmove"):{
				int xy = do_genmove();
				int x = xy/8, y = xy%8;
				do_play(x,y);
				B.show_board(myerr);
				sprintf(out,"genmove %d %d",x,y);
				fprintf(myerr,"\n") ;
				return true;
			}
			case my_hash("undo"):
				do_undo();
				sprintf(out,"undo");
				return true;
			case my_hash("validmove"):{
				int xy[64], *xyED(xy) ;
				xyED = do_validmove(xy) ;
				if( xy == xyED ){
					fprintf(myerr, "No valid move. Pass.\n") ;
					sprintf(out,"validmove");
					return true ;
				}
				int x ;
				int y ;
				do {
					--xyED ;
					x = *xyED/8 ;
					y = *xyED%8 ;
					fprintf(myerr, "(%d,%d) ", x, y) ;
				}while(xy!=xyED) ;
				fprintf(myerr, "\n") ;
				sprintf(out,"validmove");
				return true ;
			}
			case my_hash("final_score"):
				sprintf(out,"final_score %d",B.get_score());
				return true;
			case my_hash("quit"):
				sprintf(out,"quit");
				return false;

			//commmands used in simple_http_UI.cpp
			case my_hash("playgen"):{
				int x,y;
				sscanf(cmd,"%*s %d %d",&x,&y);
				do_play(x,y);
				if(B.is_game_over()==0){
					int xy = do_genmove();
					x = xy/8, y = xy%8;
					do_play(x,y);
				}
				B.show_board(myerr);
				sprintf(out,"playgen %d %d",x,y);
				return true;
			}
			case my_hash("undoundo"):{
				do_undo();
				do_undo();
				sprintf(out,"undoundo");
				return true;
			}
			case my_hash("code"):
				do_init();
				B = board(cmd+5,cmd+strlen(cmd));
				B.show_board(myerr);
				sprintf(out,"code");
				return true;
			default:
				sprintf(out,"unknown command");
				return true;
		}
	}
	std::string get_html(unsigned,unsigned)const;
};
