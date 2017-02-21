#ifndef __board__
#define __board__
#include<cctype>
#include<cstdio>
#include<algorithm>

const bool BLACK = false ;
const bool WHITE = true ;

class board{
	// my variables
	// black = X (0)
	// white = O (1)
	unsigned long long black, white ;
	bool my_tile, op_tile ;
	int pass ;

	public:

	// Construct board.
	// my_tile = BLACK
	// op_tile = WHITE
	constexpr board(): black(34628173824), white(68853694464), my_tile(false), op_tile(true), pass(0) {}

	// Input: b = black board
	// 	  w = white board
	// 	  turn = whose turn now
	// 	  _pass = pass count
	board(const unsigned long long b, const unsigned long long w, const bool turn, const int _pass){
		black = b ;
		white = w ;
		pass = _pass ;

		if( turn == BLACK ){
			my_tile = BLACK ;
			op_tile = WHITE ;
		}
		else if( turn == WHITE ){
			my_tile = WHITE ;
			op_tile = BLACK ;
		}
		else
			*this = board() ;
	}

	// Construct board from code.
	board(const char*st,const char*ed){
		black = 0 ;
		white = 0 ;
		if(ed-st==66&&std::all_of(st,ed,isdigit)){
			for(int i=0;i<64;i++){
				if( (*(st++)-'0')%3 == 1 ) // black
					black |= (1ULL<<i) ;
				else if( (*(st++)-'0')%3 == 2 ) // white
					white |= (1ULL<<i) ;
			}
			my_tile = 2-(*(st++)-'0')%2 - 1;
			op_tile = !my_tile ;
			pass = (*(st++)-'0')%3;
		}else{
			*this = board();
		}
	}

	// Input: pos = the position of the update move (presented in bitboard)
	void update(unsigned long long pos){

		if( pos ){
			unsigned long long *my_board, *op_board ;
			if( my_tile ){
				my_board = &white ;
				op_board = &black ;
			}
			else {
				my_board = &black ;
				op_board = &white ;
			}

			*my_board |= pos ;
			*op_board &= ~pos ;

			unsigned long long p ;
			unsigned char yp, ypos ;
			unsigned long long tmp_my_board, tmp_op_board ;
			ypos = 1<<(__builtin_ctzll(pos)%8) ;  // ypos = 1<<y

			// up
			p = pos ;
			tmp_my_board = *my_board ;
			tmp_op_board = *op_board ;
			do{
				tmp_my_board |= p ;
				tmp_op_board &= ~p ;
				p = p>>8 ;
			}while( *op_board & p ) ;
			if( *my_board & p ){
				*my_board = tmp_my_board ;
				*op_board = tmp_op_board ;
			}

			// up right
			p = pos ;
			yp = ypos ;
			tmp_my_board = *my_board ;
			tmp_op_board = *op_board ;
			do {
				tmp_my_board |= p ;
				tmp_op_board &= ~p ;
				p = p>>7 ;
				yp = yp<<1 ;
			}while( *op_board & p && yp ) ;
			if( *my_board & p && yp ){
				*my_board = tmp_my_board ;
				*op_board = tmp_op_board ;
			}

			// right
			p = pos ;
			yp = ypos ;
			tmp_my_board = *my_board ;
			tmp_op_board = *op_board ;
			do {
				tmp_my_board |= p ;
				tmp_op_board &= ~p ;
				p = p<<1 ;
				yp = yp<<1 ;
			}while( *op_board & p && yp ) ;
			if( *my_board & p && yp ){
				*my_board = tmp_my_board ;
				*op_board = tmp_op_board ;
			}
	
			// down right
			p = pos ;
			yp = ypos ;
			tmp_my_board = *my_board ;
			tmp_op_board = *op_board ;
			do {
				tmp_my_board |= p ;
				tmp_op_board &= ~p ;
				p = p<<9 ;
				yp = yp<<1 ;
			}while( *op_board & p && yp ) ;
			if( *my_board & p && yp ){
				*my_board = tmp_my_board ;
				*op_board = tmp_op_board ;
			}
			
			// down
			p = pos ;
			tmp_my_board = *my_board ;
			tmp_op_board = *op_board ;
			do {
				tmp_my_board |= p ;
				tmp_op_board &= ~p ;
				p = p<<8 ;
			}while( *op_board & p ) ;
			if( *my_board & p ){
				*my_board = tmp_my_board ;
				*op_board = tmp_op_board ;
			}
			
			// down left
			p = pos ;
			yp = ypos ;
			tmp_my_board = *my_board ;
			tmp_op_board = *op_board ;
			do {
				tmp_my_board |= p ;
				tmp_op_board &= ~p ;
				p = p<<7 ;
				yp = yp>>1 ;
			}while( *op_board & p && yp ) ;
			if( *my_board & p && yp ){
				*my_board = tmp_my_board ;
				*op_board = tmp_op_board ;
			}
			
			// left
			p = pos ;
			yp = ypos ;
			tmp_my_board = *my_board ;
			tmp_op_board = *op_board ;
			do {
				tmp_my_board |= p ;
				tmp_op_board &= ~p ;
				p = p>>1 ;
				yp = yp>>1 ;
			}while( *op_board & p && yp ) ;
			if( *my_board & p && yp ){
				*my_board = tmp_my_board ;
				*op_board = tmp_op_board ;
			}
			
			// up left
			p = pos ;
			yp = ypos ;
			tmp_my_board = *my_board ;
			tmp_op_board = *op_board ;
			do {
				tmp_my_board |= p ;
				tmp_op_board &= ~p ;
				p = p>>9 ;
				yp = yp>>1 ;
			}while( *op_board & p && yp ) ;
			if( *my_board & p && yp ){
				*my_board = tmp_my_board ;
				*op_board = tmp_op_board ;
			}
			
			pass = 0;
		}
		else {
			++pass;
		}

		// player swap
		bool tmp_tile = my_tile ;
		my_tile = op_tile ;
		op_tile = tmp_tile ;
	}

	// Input: x = x-axis of the move 
	// 	  y = y-axis of the move
	void update(int x,int y){
		if( x == 8 && y == 0 )
			update(0) ;
		else
			update(1ULL<<(x*8+y)) ;
	}


	// Input: b = black board
	// 	  w = white board
	// 	  _pass = whether this time is pass or not
	void undo(const unsigned long long b, const unsigned long long w, int _pass){
		black = b ;
		white = w ;
		pass = _pass ;
		
		// player swap
		bool tmp_tile = my_tile ;
		my_tile = op_tile ;
		op_tile = tmp_tile ;
	}

	bool is_valid_move_new(const int x, const int y){
		static bool*** table = construct_valid_move_table() ;
		
		unsigned long long my_board, op_board ;
		if( my_tile ){
			my_board = white ;
			op_board = black ;
		}
		else {
			my_board = black ;
			op_board = white ;
		}

		const unsigned long long row_mask = 255 ;
	//	const unsigned long long col_mask = 72340172838076673 ;

		// Check row
		int shift = 8*y ;
		if( table[ (unsigned char)((my_board>>shift)&row_mask) ][ (unsigned char)((op_board>>shift)&row_mask) ][x] )
			return true ;
		
		// Check column
		
		// Check up-left - down-right
		
		// Check up-right - down-left

		return false ;
	}

	bool*** construct_valid_move_table(){
		bool*** table = new bool**[256] ;
		for(int i = 0 ; i < 256 ; i++){
			table[i] = new bool*[256] ;
			for(int j = 0 ; j < 256 ; j++)
				table[i][j] = new bool[8] ;
		}

		for(unsigned char my = 0 ; my < 256 ; my++){
			for(unsigned char op = 0 ; op < 256 ; op++){
				unsigned char pos = 1 ;
				for(int p = 0 ; p < 8 ; p++, pos = pos<<1){

					table[my][op][p] = false ;

					if( (my&op) || (my&pos) || (op&pos) )
						continue ;

					// right
					unsigned char tmppos = pos<<1 ;
					if( op & tmppos ){
						do {
							tmppos = tmppos<<1 ;
						}while( op & tmppos ) ;
						if( my & tmppos ){
							table[my][op][p] = true ;
							continue ;
						}
					}
		
					// left
					tmppos = pos>>1 ;
					if( op & tmppos ){
						do {
							tmppos = tmppos>>1 ;
						}while( op & tmppos ) ;
						if( my & tmppos ){
							table[my][op][p] = true ;
							continue ;
						}
					}
				}
			}
		}

		return table ;
	}
	
	// Input: pos = the position of the move (presented in bitboard)
	// 	  ypos = the y-axis of the move (presented in bitboard)
	bool is_valid_move(const unsigned long long pos, const unsigned char ypos)const{

		if( black & pos || white & pos )
			return false ;

		if( pos ){
			unsigned long long my_board, op_board ;
			if( my_tile ){
				my_board = white ;
				op_board = black ;
			}
			else {
				my_board = black ;
				op_board = white ;
			}
			my_board |= pos ;
			op_board &= ~pos ;

			unsigned long long p ;
			unsigned char yp ;

			// up
			p = pos>>8 ;
			if( op_board & p ){
				do{
					p = p>>8 ;
				}while( op_board & p ) ;
				if( my_board & p )
					return true ;
			}

			// up right
			p = pos>>7 ;
			yp = ypos<<1 ;
			if( op_board & p && yp ){
				do {
					p = p>>7 ;
					yp = yp<<1 ;
				}while( op_board & p && yp ) ;
				if( my_board & p && yp )
					return true ;
			}

			// right
			p = pos<<1 ;
			yp = ypos<<1 ;
			if( op_board & p && yp ){
				do {
					p = p<<1 ;
					yp = yp<<1 ;
				}while( op_board & p && yp ) ;
				if( my_board & p && yp )
					return true ;
			}
	
			// down right
			p = pos<<9 ;
			yp = ypos<<1 ;
			if( op_board & p && yp ){
				do {
					p = p<<9 ;
					yp = yp<<1 ;
				}while( op_board & p && yp ) ;
				if( my_board & p && yp )
					return true ;
			}
			
			// down
			p = pos<<8 ;
			if( op_board & p ){
				do {
					p = p<<8 ;
				}while( op_board & p ) ;
				if( my_board & p )
					return true ;
			}
			
			// down left
			p = pos<<7 ;
			yp = ypos>>1 ;
			if( op_board & p && yp ){
				do {
					p = p<<7 ;
					yp = yp>>1 ;
				}while( op_board & p && yp ) ;
				if( my_board & p && yp )
					return true ;
			}
			
			// left
			p = pos>>1 ;
			yp = ypos>>1 ;
			if( op_board & p && yp ){
				do {
					p = p>>1 ;
					yp = yp>>1 ;
				}while( op_board & p && yp ) ;
				if( my_board & p && yp )
					return true ;
			}
			
			// up left
			p = pos>>9 ;
			yp = ypos>>1 ;
			if( op_board & p && yp ){
				do {
					p = p>>9 ;
					yp = yp>>1 ;
				}while( op_board & p && yp ) ;
				if( my_board & p && yp )
					return true ;
			}
		}

		return false ;
	}

	// Input: x = x-axis of the move 
	// 	  y = y-axis of the move
	bool is_valid_move(int x,int y)const{
		if( x == 8 && y == 0 ){ 
			unsigned long long b[64] ;
			return b == get_valid_move(b) ;
		}
		else {
			unsigned long long pos = 1ULL<<(x*8+y) ;
			unsigned char ypos = 1<<y ;
			return is_valid_move(pos, ypos) ;
		}
	}

	// Input: val = the pointer of valid move array
	unsigned long long* get_valid_move(unsigned long long* val)const{

		unsigned long long p = 1 ;
		unsigned char yp = 1 ;
		do{
			if( (~black & p || ~white & p) && is_valid_move(p, yp) ){
				*val = p ;
				++val ;
			}

			p = p<<1 ;
			yp = yp<<1 ;
			if(!yp)
				yp = 1 ;
		}while(p) ;

		return val ;
	}

	// Input: val = the pointer of valid move array
	int* get_valid_move(int* val)const{

		unsigned long long p = 1 ;
		unsigned char yp = 1 ;
		do{
			if( (~black & p || ~white & p) && is_valid_move(p, yp) ){
				*val = __builtin_ctzll(p) ;
				++val ;
			}

			p = p<<1 ;
			yp = yp<<1 ;
			if(!yp)
				yp = 1 ;
		}while(p) ;

		return val ;
	}

	void show_board(FILE*fp)const{
		static constexpr char c[]{'.','X','O'};
		if(my_tile)
        		fprintf(fp,"O's turn\n") ;
		else
        		fprintf(fp,"X's turn\n") ;

		fprintf(fp,"| ");
		for(int j=0;j!=8;++j){
			fprintf(fp,"|%c",'0'+j);
		}
		fprintf(fp,"|\n");

		unsigned long long p = 1 ;
		for(int i=0;i!=8;++i){
			fprintf(fp,"|%c",'0'+i);
			for(int j=0;j!=8;++j){
				if( black & p )
					fprintf(fp,"|%c",c[1]);
				else if( white & p )
					fprintf(fp,"|%c",c[2]);
				else
					fprintf(fp,"|%c",c[0]);
				p = p<<1 ;
			}
			fprintf(fp,"|\n");
		}
		fflush(fp);
	}

	std::pair<int,int> get_count()const{
		return std::pair<int,int>(__builtin_popcountll(black), __builtin_popcountll(white));
	}

	int get_score()const{
		return __builtin_popcountll(black) - __builtin_popcountll(white) ;
	}

	const bool operator[](int x)const{
		if( black & (1ULL<<x) )
			return BLACK ;
		else
			return WHITE ;
	}

	unsigned long long get_black()const{
		return black ;
	}

	unsigned long long get_white()const{
		return white ;
	}

	int get_pass()const{
		return pass;
	}

	bool get_my_tile()const{
		return my_tile;
	}

	bool is_game_over()const{
		return pass==2;
	}
};
#endif
