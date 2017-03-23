#include"board.h"
#include"openbook.h"
#include<random>
#ifdef _WIN32
#include<chrono>
#endif

#ifdef _WINDOWS
#include<windows.h>
#else
#include<ctime>
#endif

#include<cstring>
#include<string>
#include<sys/time.h>
#include<signal.h>
#include<cmath>
#include<utility>

// parameters
constexpr double UCB_c = 0.1 ;
constexpr int simulateN = 10000 ;

constexpr int OpenBookDepth = 14 ;
constexpr int SearchDepth = 20 ;

constexpr int SimulateTime = 30 ;
constexpr int preSimulateTime = 1 ;
constexpr int SearchTime = SimulateTime - preSimulateTime ;
constexpr clock_t PreservedTime = 60 ;

//constexpr int HashTableSize = 1<<26 ;

// global counters
long long int totalsim = 0 ;
clock_t TotalTimeLimit = 1800 * CLOCKS_PER_SEC ;

// globla variables
#ifdef _WINDOWS
DWORD Tick ;
int TimeOut ;
#else
clock_t Tick ;
clock_t TimeOut ;
#endif
/*
HashInfo *HashTable ;
unsigned long long int HashPos[2][64] ;
unsigned long long int HashColor[2] ;
*/

inline bool TimesUp(){
#ifdef _WINDOWS
	return ( GetTickCount() - Tick >= TimeOut ) ;
#else
	return ( clock() - Tick >= TimeOut ) ;
#endif
}

void SetClock(int alarm_time){
#ifdef _WINDOWS
	Tick = GetTickCount() ;
	TimeOut = alarm_time * 1000 ;
#else
	Tick = clock() ;
	TimeOut = alarm_time * CLOCKS_PER_SEC ;
#endif
	return ;
}

constexpr char m_tolower(const char c){
	return c+('A'<=c&&c<='Z')*('a'-'A');
}

constexpr unsigned my_hash(const char*s,unsigned long long int hv=0){
	return *s&&*s!=' '?my_hash(s+1,(hv*('a'+1)+m_tolower(*s))%0X3FFFFFFFU):hv;
}

constexpr int sign(const int x){
	return (x > 0) - (x < 0) ;
}

constexpr double var(double winrate){
	return (winrate * (1-winrate)) ;
}

constexpr double stddev(double winrate){
	return sqrt(winrate * (1-winrate)) ;
}

struct HashInfo {
	unsigned long long hash ;
	int score ;
	std::pair<int,int> pos ;

	constexpr HashInfo(): hash(0), score(0), pos() {} ;
} ;

struct history {
	unsigned long long black ;
	unsigned long long white ;
	int pass ;
//	unsigned long long hash ;
} ;

struct grade {
	long long int win ;
	long long int lose ;
	long long int draw ;
	long long int simulateCount ;

	constexpr grade(): win(0), lose(0), draw(0), simulateCount(0) {}
} ;

struct node {
	long long int win ;
	long long int lose ;
	long long int draw ;
	long long int simulateCount ;
	double winrate ;
	double pot ;

	board B ;
	std::pair<int,int> pos ;

	int childCount ;
	struct node *child ;
	struct node *next ;

	constexpr node(): 
	win(0), lose(0), draw(0), simulateCount(0), 
	winrate(0), pot(0), B(), pos(),
	childCount(0), child(NULL), next(NULL) {}

	constexpr node(board _B): 
	win(0), lose(0), draw(0), simulateCount(0), 
	winrate(0), pot(0), B(_B), pos(),  
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
		
		if( no_valid_move() )
			return std::pair<int,int>(8,0) ;
		
		bool my_tile = B.get_my_tile() ;
		int depth = 64 - B.get_count() ;

		if( B.get_count() <= OpenBookDepth ){
			std::pair<int,int> pos = OpenBook(B.get_black(), B.get_white()) ;
			if( B.is_valid_move(pos.first, pos.second) )
				return pos ;
		}

		if( depth <= SearchDepth ){
			// Monte-Carlo
			SetClock(preSimulateTime) ;
			
			node *root = new node(B) ;
			while( !TimesUp() ){
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

			std::pair<int,int> MCBestMove = maxN->pos ;
			fprintf(stderr,"Totally %lld simulations\n", totalsim) ;
			totalsim = 0 ;
			destroyTree(root) ;

			// Search
			SetClock(SearchTime) ;
			
			fprintf(stderr, "Search at depth %d: Start searching...\n", depth) ;
			std::pair<int,int>SBestMove = SearchBestMove(B) ;
			if( TimesUp() ){
				fprintf(stderr, "Search: Time Limit Exceed\n") ;
				SetClock(0) ;
				return MCBestMove ;
			}
			else {
				SetClock(0) ;
				return SBestMove ;
			}
		}
		else {
			return do_ranplay() ;

			// Monte-Carlo
			SetClock(SimulateTime) ;

			node *root = new node(B) ;
			while( !TimesUp() ){
				grade g = root_simulate(root, my_tile, my_tile) ;
				root->simulateCount += g.simulateCount ;
			}

			node *maxN = root->child ;
			node *tmpN = root->child->next ;
			fprintf(stderr, "(%d,%d): %f with %lld simulations\n", maxN->pos.first, maxN->pos.second, maxN->winrate, maxN->simulateCount);
			for( int i = 1 ; i < root->childCount ; ++i ){
				fprintf(stderr, "(%d,%d): %f with %lld simulations\n", tmpN->pos.first, tmpN->pos.second, tmpN->winrate, tmpN->simulateCount);
				if( tmpN->winrate > maxN->winrate )
					maxN = tmpN ;
				tmpN = tmpN->next ;
			}

			std::pair<int,int> BestMove = maxN->pos ;
			fprintf(stderr,"Totally %lld simulations\n", totalsim) ;
			totalsim = 0 ;
			destroyTree(root) ;
			SetClock(0) ;
			return BestMove ;
		}
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

	// All information is updated at child level
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
			double pot = sqrt(log(root->simulateCount + g.simulateCount)/log(root->simulateCount)) ;
			tmpN = root->child ;
			for( int i = 0 ; i < root->childCount ; ++i ){
				tmpN->pot *= pot ;
				tmpN = tmpN->next ;
			}
			
			if( my_tile == top_root_tile ){
				maxN->win += g.win ;
				maxN->lose += g.lose ;
			}
			else {
				maxN->win += g.lose ;
				maxN->lose += g.win ;
			}
			maxN->draw += g.draw ;
			maxN->simulateCount += g.simulateCount ;
			maxN->winrate = double(maxN->win)/maxN->simulateCount ;
			maxN->pot = UCB_c*sqrt(var(maxN->winrate)*log(root->simulateCount + g.simulateCount)/maxN->simulateCount) ;
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

			long long int simulateCount = simulateN/nodeCount ;
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
					tmpN->pot *= stddev(tmpN->winrate) ;
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
					tmpN->pot *= stddev(tmpN->winrate) ;
					tmpN = tmpN->next ;
					
					g.win += tmpg.win ;
					g.lose += tmpg.lose ;
					g.draw += tmpg.draw ;
				}
			}

			g.simulateCount = simulateCount*nodeCount ;
		}

		// Back propagation
		return g ;
	}

	grade leaf_simulate(board B, bool top_root_tile, long long int simulateCount){
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
			if( top_root_tile ){
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
			int t = -Search(tmpB, -beta, -MaxScore) ;
			if( t > MaxScore ){
				MaxScore = t ;
				BestMove = ML[i] ;
			}
			if( MaxScore >= beta ){
				fprintf(stderr, "Search result: Win\n") ;
				return BestMove ;
			}
		}

		if( MaxScore < 0 )
			fprintf(stderr, "Search result: Lose\n") ;
		else if( MaxScore == 0 )
			fprintf(stderr, "Search result: Draw\n") ;
		return BestMove ;
	}

	int Search(board B, const int alpha, const int beta){
		if( B.is_game_over() || TimesUp() )
			return sign(B.get_my_score()) ;

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
	
	//	HashTable = new HashInfo[HashTableSize] ;
	//	HashPos[2][64] ;
	//	HashColor[2] ;
		
#ifdef _WINDOWS
		srand(Tick = GetTickCount()) ;
#else
		srand(Tick = time(NULL)) ;
#endif
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
			case my_hash("showboard"):{ 
				B.show_board(myerr);
				sprintf(out,"showboard");
				return true;
			}
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
				clock_t start_time = clock() ;
				std::pair<int,int> xy = do_genmove();
				int x = xy.first ;
				int y = xy.second;
				do_play(x, y);
				B.show_board(myerr);
				sprintf(out,"genmove %d %d",x,y);
				fprintf(myerr,"\n") ;
				TotalTimeLimit -= (clock() - start_time) ;
				fprintf(stderr, "Time left: %ld seconds\n", TotalTimeLimit/CLOCKS_PER_SEC) ;
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
