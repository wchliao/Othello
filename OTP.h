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
#include<utility>

// parameters
const double UCB_c = 0.8 ;
const int simulateN = 100 ;
const int SearchDepth = 5 ;
const int alarm_time = 3 ;

// global flags
bool stopflag = false ;

// global counters
int totalsim = 0 ;

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

	constexpr grade(): win(0), lose(0), draw(0), simulateCount(0) {}
} ;

struct node {
	int win ;
	int lose ;
	int draw ;
	double winrate ;
	double pot ;
	board B ;
	std::pair<int,int> pos ;

	int simulateCount ;

	int childCount ;
	struct node *child ;
	struct node *next ;

	constexpr node(): 
	win(0), lose(0), draw(0), winrate(0), pot(0), 
	B(), pos(), simulateCount(0),
	childCount(0), child(NULL), next(NULL) {}

	constexpr node(board _B): 
	win(0), lose(0), draw(0), winrate(0), pot(0), 
	B(_B), pos(), simulateCount(0), 
	childCount(0), child(NULL), next(NULL) {}
} ;

void destroyTree(node *root){
	if( root->childCount > 0 ){
		node *cur = root->child ;
		node *next = cur->next ;
		destroyTree(cur) ;
		for( int i = 1 ; i < root->childCount ; ++i ){
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

	// Randomly return a valid move
	std::pair<int,int> do_ranplay(){
		if( B.is_game_over() )
			return std::pair<int,int>(8,0) ; // pass 
		else {
			std::pair<int,int> ML[64], *MLED(B.get_valid_move(ML)) ;
			return *random_choice(ML,MLED) ;
		}
	}

	//choose the best move in do_genmove
	std::pair<int,int> do_genmove(){
		struct itimerval time ;
		time.it_interval.tv_usec = 0 ;
		time.it_interval.tv_sec = 0 ;
		time.it_value.tv_usec = 0 ;
		time.it_value.tv_sec = alarm_time ;
		if( setitimer( ITIMER_REAL, &time, NULL) < 0 )
			fprintf(stderr, "set timer fail.\n") ;

		stopflag = false ;

		std::pair<int,int> ans = genmove() ;
		fprintf(stderr,"totalsim = %d\n", totalsim);
		totalsim = 0 ;

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

	std::pair<int,int> genmove(){
		if( no_valid_move() )
			return std::pair<int,int>(8,0) ;

		bool my_tile = B.get_my_tile() ;
		// Search
//		int depth = 64 - __builtin_popcountll(B.get_black() | B.get_white()) ;
//		if( depth < SearchDepth ){
//			fprintf(stderr, "Search: Start searching...\n") ;
//			std::pair<int,int>BestMove = SearchBestMove(B) ;
//			if( stopflag )
//				fprintf(stderr, "Search: Time Limit Exceed\n") ;
//			return BestMove ;
//		}

		// Monte-Carlo
		node *root = new node(B) ;
		while(!stopflag){
			grade g = root_simulate(root, my_tile, my_tile) ;
			root->simulateCount += g.simulateCount ;
		}

		node *maxN = root->child ;
		node *tmpN = root->child->next ;
		for( int i = 1 ; i < root->childCount ; ++i ){
			if( tmpN->winrate > maxN->winrate )
				maxN = tmpN ;
			tmpN = tmpN->next ;
		}

		destroyTree(root) ;
		return maxN->pos ;
	}

	grade root_simulate(node *root, bool my_tile, bool top_root_tile){
		grade g ;

		if( root->B.is_game_over() ){
			int score = root->B.get_score() ;
			if( top_root_tile ){
				if( score > 0 )
					g.lose = simulateN ;
				else if( score < 0 )
					g.win = simulateN ;
				else
					g.draw = simulateN ;
			}
			else {
				if( score > 0 )
					g.win = simulateN ;
				else if( score < 0 )
					g.lose = simulateN;
				else
					g.draw = simulateN ;
			}
			g.simulateCount = simulateN ;
			root->simulateCount += g.simulateCount ;
			totalsim += simulateN ;
			return g ;
		}
		
		// Selection 
		node *tmpN ;
		if( root->childCount > 0 ){
			node *maxN = root->child ;
			tmpN = maxN->next ;
			for( int i = 1 ; i < root->childCount ; ++i ){
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
			}
			else {
				maxN->win += g.lose ;
				maxN->lose += g.win ;
				maxN->draw += g.draw ;
			}
			maxN->winrate = double(maxN->win)/maxN->simulateCount ;
			maxN->pot *= sqrt((double)(maxN->simulateCount - g.simulateCount) / maxN->simulateCount) ;
			
			double pot = sqrt(log(root->simulateCount + g.simulateCount)/log(root->simulateCount)) ;
			tmpN = root->child ;
			for( int i = 0 ; i < root->childCount ; ++i ){
				tmpN->pot *= pot ;
				tmpN = tmpN->next ;
			}
		}
		else {
			std::pair<int,int> ML[64], *MLED(root->B.get_valid_move(ML)) ;
			int nodeCount = MLED - ML ;

			// Expansion
			if( nodeCount == 0 ){
				nodeCount = 1 ;
				root->child = new node(root->B) ;
				root->childCount = nodeCount ;
				root->child->pos = std::pair<int,int>(8,0) ;
			}
			else {
				root->child = new node(root->B) ;
				root->childCount = nodeCount ;
				tmpN = root->child ;
				for( int i = 1 ; i < nodeCount ; ++i ){
					tmpN->next = new node(root->B) ;
					tmpN = tmpN->next ;
				}
				root->child->pos = ML[0] ;
			}

			int simulateCount = simulateN/nodeCount ;
			double pot = UCB_c*sqrt(log(simulateCount*nodeCount)/simulateCount) ;
			root->child->pot = pot ;
			root->child->simulateCount = simulateCount ;
			root->child->B.update(root->child->pos) ;

			tmpN = root->child ;
			for( int i = 1 ; i < nodeCount ; ++i ){
				tmpN->next->pos = ML[i] ;
				tmpN->next->pot = pot ;
				tmpN->next->simulateCount = simulateCount ;
				tmpN->next->B.update(tmpN->next->pos) ;
				tmpN = tmpN->next ;
			}
			tmpN->next = NULL ;

			// Simulations
			grade tmpg ;
			tmpN = root->child ;
			if( my_tile == top_root_tile ){
				for( int i = 0 ; i < nodeCount ; ++i ){
					tmpg = leaf_simulate(tmpN->B, top_root_tile, simulateCount) ;
					tmpN->win = tmpg.win ;
					tmpN->lose = tmpg.lose ;
					tmpN->draw = tmpg.draw ;
					tmpN->winrate = double(tmpN->win)/tmpN->simulateCount ;
					tmpN = tmpN->next ;

					g.win += tmpg.win ;
					g.lose += tmpg.lose ;
					g.draw += tmpg.draw ;
				}
			}
			else {
				for( int i = 0 ; i < nodeCount ; ++i ){
					tmpg = leaf_simulate(tmpN->B, top_root_tile, simulateCount) ;
					tmpN->win = tmpg.lose ;
					tmpN->lose = tmpg.win ;
					tmpN->draw = tmpg.draw ;
					tmpN->winrate = double(tmpN->win)/tmpN->simulateCount ;
					tmpN = tmpN->next ;
					
					g.win += tmpg.win ;
					g.lose += tmpg.lose ;
					g.draw += tmpg.draw ;
				}
			}

			g.simulateCount = simulateCount*nodeCount ;
			root->simulateCount += g.simulateCount ;
		}

		// Back propagation
		return g ;
	}

	grade leaf_simulate(board B, bool my_tile, int simulateCount){
		grade g ;
		std::pair<int,int> ML[64], *MLED(ML) ;
		int score ;

		for( int t = 0 ; t < simulateCount ; ++t ){
			board tmpB = B ;
			while(!tmpB.is_game_over()){
				MLED = tmpB.get_valid_move(ML) ;
				if( ML == MLED )
					tmpB.update(8,0) ; // pass
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
			}
			else {
				if( score > 0 )
					++g.win ;
				else if( score < 0 )
					++g.lose ;
				else
					++g.draw ;
			}
		}
		g.simulateCount = simulateCount ;
		totalsim += simulateCount ;
		return g ;
	}

	std::pair<int,int> SearchBestMove(board B){
		std::pair<int,int> ML[64], *MLED(B.get_valid_move(ML)) ;
		int nodeCount = MLED - ML ;
		if( nodeCount == 0 )
			return std::pair<int,int>(8,0) ; // pass
			
		const int beta = 1 ;
		
		std::pair<int,int> BestMove = ML[0] ; 
		int MaxScore = -1 ;

		for( int i = 0 ; i < nodeCount ; ++i ){
			board tmpB = B ;
			tmpB.update(ML[i]) ;
			int t = -Search(B, -beta, -MaxScore) ;
			if( t > MaxScore ){
				MaxScore = t ;
				BestMove = ML[i] ;
			}
			if( MaxScore >= beta )
				break ;
		}

		return BestMove ;
	}

	int Search(board B, int alpha, int beta){
		if( B.is_game_over() ){
			bool my_tile = B.get_my_tile() ;
			
			int score = B.get_score() ;
			if( score > 0 )
				score = 1 ;
			else if( score < 0 )
				score = -1 ;

			if( my_tile )
				return -score ;
			else 
				return score ;
		}	

		std::pair<int,int> ML[64], *MLED(ML) ;
		int m = alpha ;

		MLED = B.get_valid_move(ML) ;
		int nodeCount = MLED - ML ;
		if( nodeCount == 0 ){
			board tmpB = B ;
			tmpB.update(8,0) ; // pass
			return -Search(tmpB, -beta, -m) ;
		}
		else {
			for(int i = 0 ; i < nodeCount ; ++i){
				board tmpB = B ;
				tmpB.update(ML[i]) ;
				int t = -Search(tmpB, -beta, -m) ;
				if( t > m )
					m = t ;
				if( m >= beta )
					return m ;
			}
		}

		return m ;
	}

	bool no_valid_move(){
		std::pair<int,int> ML[64], *MLED(B.get_valid_move(ML)) ;
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
				sprintf(out,"name Wei_Chung_Liao");
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
			case my_hash("ranplay"):{
				std::pair<int,int>xy = do_ranplay() ;
				int x = xy.first ;
				int y = xy.second ;
				do_play(x, y) ;
				B.show_board(myerr) ;
				sprintf(out,"ranplay %d %d",x,y);
				fprintf(myerr,"\n") ;
				return true;
			}
			case my_hash("genmove"):{
				std::pair<int,int> xy = do_genmove();
				int x = xy.first ;
				int y = xy.second;
				do_play(x, y);
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
				std::pair<int,int> ML[64], *MLED(ML), *pML(ML) ;
				MLED = B.get_valid_move(ML) ;
				if( pML == MLED ){
					fprintf(myerr, "No valid move. Pass.\n") ;
					sprintf(out,"validmove");
					return true ;
				}
				do {
					fprintf(myerr, "(%d,%d) ", pML->first, pML->second) ;
					++pML ;
				}while(pML!=MLED) ;
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
					std::pair<int,int> xy = do_genmove();
					do_play(xy.first, xy.second);
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
