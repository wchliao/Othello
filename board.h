#ifndef __board__
#define __board__
#include<cctype>
#include<cstdio>
#include<utility>
#include<algorithm>
#include<iostream>
#include "table.h"

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

	void update(int x, int y){
		static UpdateTable table = UpdateTable() ;

		if( x < 8 ){
			unsigned long long *my_board, *op_board ;
			if( my_tile ){
				my_board = &white ;
				op_board = &black ;
			}
			else {
				my_board = &black ;
				op_board = &white ;
			}

			unsigned long long origin_my = *my_board ;
			unsigned long long origin_op = *op_board ;
			
			static const unsigned long long row_mask = 	0xFF ;
			static const unsigned long long col_mask = 	0x0101010101010101 ;
			static const unsigned long long mplus_mask = 	0x0102040810204080 ;
			static const unsigned long long mminus_mask = 	0x8040201008040201 ;
			static const unsigned long long col_ones[] = {0, 0x0101010101010101, 
				0x0303030303030303, 0x0707070707070707, 0x0F0F0F0F0F0F0F0F,
				0x1F1F1F1F1F1F1F1F, 0x3F3F3F3F3F3F3F3F, 0x7F7F7F7F7F7F7F7F,
				0xFFFFFFFFFFFFFFFF } ;

			static const unsigned long long col_magic = mplus_mask ;
			static const unsigned long long m_magic = col_mask ;
	
			// Update row
			int shift = x<<3 ;
			unsigned long long my = (origin_my>>shift)&row_mask ;
			unsigned long long op = (origin_op>>shift)&row_mask ;

			unsigned long long update_my = table.get_value(0, my, op, y).first ;
			unsigned long long update_op = table.get_value(0, my, op, y).second ;
			
			*my_board = *my_board & ~(row_mask<<shift) ;
			*my_board = *my_board | (update_my<<shift) ;
			*op_board = *op_board & ~(row_mask<<shift) ;
			*op_board = *op_board | (update_op<<shift) ;

			// Update column
			my = origin_my>>y ;
			my = my & col_mask ;
			my = my * col_magic ;
			my = my>>56 ;

			op = origin_op>>y ;
			op = op & col_mask ;
			op = op * col_magic ;
			op = op>>56 ;

			update_my = table.get_value(1, my, op, x).first ;
			update_op = table.get_value(1, my, op, x).second ;
		
			*my_board = *my_board & ~(col_mask<<y) ;
			*my_board = *my_board | (update_my<<y) ;
			*op_board = *op_board & ~(col_mask<<y) ;
			*op_board = *op_board | (update_op<<y) ;
			
			// Update slope where m > 0
			if( x + y > 7 ){
				shift = (x+y-7)<<3 ; // (x+y-8)*8

				my = origin_my>>shift ;
				my = my & mplus_mask ;
				my = my * m_magic ;
				my = my>>56 ;

				op = origin_op>>shift ;
				op = op & mplus_mask ;
				op = op * m_magic ;
				op = op>>56 ;

				update_my = table.get_value(2, my, op, y).first ;
				update_op = table.get_value(2, my, op, y).second ;
		
				*my_board = *my_board & ~(mplus_mask<<shift) ;
				*my_board = *my_board | (update_my<<shift) ;
				*op_board = *op_board & ~(mplus_mask<<shift) ;
				*op_board = *op_board | (update_op<<shift) ;
			}
			else {
				shift = 7-x-y ; 

				my = origin_my<<shift ;
				my = my & mplus_mask ;
				my = my * m_magic ;
				my = my>>56 ;
				my = my & ~col_ones[shift] ;

				op = origin_op<<shift ;
				op = op & mplus_mask ;
				op = op * m_magic ;
				op = op>>56 ;
				op = op & ~col_ones[shift] ;

				update_my = table.get_value(2, my, op, y+shift).first ;
				update_op = table.get_value(2, my, op, y+shift).second ;
		
				*my_board = *my_board & ~((mplus_mask & ~col_ones[shift])>>shift) ;
				*my_board = *my_board | (update_my>>shift) ;
				*op_board = *op_board & ~((mplus_mask & ~col_ones[shift])>>shift) ;
				*op_board = *op_board | (update_op>>shift) ;
			}
		
			// Update slope where m < 0
			if( x < y ){
				shift = (y-x)<<3 ; // (y-x)*8

				my = origin_my<<shift ;
				my = my & mminus_mask ;
				my = my * m_magic ;
				my = my>>56 ;

				op = origin_op<<shift ;
				op = op & mminus_mask ;
				op = op * m_magic ;
				op = op>>56 ;

				update_my = table.get_value(3, my, op, y).first ;
				update_op = table.get_value(3, my, op, y).second ;
		
				*my_board = *my_board & ~(mminus_mask>>shift) ;
				*my_board = *my_board | (update_my>>shift) ;
				*op_board = *op_board & ~(mminus_mask>>shift) ;
				*op_board = *op_board | (update_op>>shift) ;
			}
			else {
				shift = x-y ; 

				my = origin_my<<shift ;
				my = my & mminus_mask ;
				my = my * m_magic ;
				my = my>>56 ;
				my = my & ~col_ones[shift] ;

				op = origin_op<<shift ;
				op = op & mminus_mask ;
				op = op * m_magic ;
				op = op>>56 ;
				op = op & ~col_ones[shift] ;

				update_my = table.get_value(3, my, op, y+shift).first ;
				update_op = table.get_value(3, my, op, y+shift).second ;
		
				*my_board = *my_board & ~((mminus_mask & ~col_ones[shift])>>shift) ;
				*my_board = *my_board | (update_my>>shift) ;
				*op_board = *op_board & ~((mminus_mask & ~col_ones[shift])>>shift) ;
				*op_board = *op_board | (update_op>>shift) ;
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

	void update(std::pair<int,int> pos){
		update(pos.first, pos.second) ;
		return ;
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

	bool is_valid_move(const int x, const int y){
		static IsValidTable table = IsValidTable() ;
		const unsigned long long ones[] = {0, 1, 3, 7, 15, 31, 63, 127, 255} ;

		if( x == 8 && y == 0 ){ 
			std::pair<int,int> b[64] ;
			return b == get_valid_move(b) ;
		}

		unsigned long long pos = (1ULL<<(x*8+y)) ;
		if( black & pos || white & pos )
			return false ;

		unsigned long long my_board, op_board ;
		if( my_tile ){
			my_board = white ;
			op_board = black ;
		}
		else {
			my_board = black ;
			op_board = white ;
		}

		const unsigned long long row_mask = 	0xFF ;
		const unsigned long long col_mask = 	0x0101010101010101 ;
		const unsigned long long mplus_mask = 	0x0102040810204080 ;
		const unsigned long long mminus_mask = 	0x8040201008040201 ;

		const unsigned long long col_magic = mplus_mask ;
		const unsigned long long m_magic = col_mask ;
		
		// Check row
		int shift = x<<3 ; // x*8
		unsigned long long my = (my_board>>shift)&row_mask ;
		unsigned long long op = (op_board>>shift)&row_mask ;
		if( table.get_value(my, op, y) )
			return true ;
		
		// Check column
		my = my_board>>y ;
		my = my & col_mask ;
		my = my * col_magic ;
		my = my>>56 ;

		op = op_board>>y ;
		op = op & col_mask ;
		op = op * col_magic ;
		op = op>>56 ;

		if( table.get_value(my, op, x) )
			return true ;
		
		// Check slope where m > 0
		if( x + y > 7 ){
			shift = (x+y-7)<<3 ; // (x+y-8)*8

			my = my_board>>shift ;
			my = my & mplus_mask ;
			my = my * m_magic ;
			my = my>>56 ;

			op = op_board>>shift ;
			op = op & mplus_mask ;
			op = op * m_magic ;
			op = op>>56 ;

			if( table.get_value(my, op, y) )
				return true ;
		}
		else {
			shift = 7-x-y ; 

			my = my_board<<shift ;
			my = my & mplus_mask ;
			my = my * m_magic ;
			my = my>>56 ;
			my = my & ~ones[shift] ;

			op = op_board<<shift ;
			op = op & mplus_mask ;
			op = op * m_magic ;
			op = op>>56 ;
			op = op & ~ones[shift] ;

			if( table.get_value(my, op, y+shift) )
				return true ;
		}
		
		// Check slope where m < 0
		if( x < y ){
			shift = (y-x)<<3 ; // (y-x)*8

			my = my_board<<shift ;
			my = my & mminus_mask ;
			my = my * m_magic ;
			my = my>>56 ;

			op = op_board<<shift ;
			op = op & mminus_mask ;
			op = op * m_magic ;
			op = op>>56 ;

			if( table.get_value(my, op, y) )
				return true ;
		}
		else {
			shift = x-y ; 

			my = my_board<<shift ;
			my = my & mminus_mask ;
			my = my * m_magic ;
			my = my>>56 ;
			my = my & ~ones[shift] ;

			op = op_board<<shift ;
			op = op & mminus_mask ;
			op = op * m_magic ;
			op = op>>56 ;
			op = op & ~ones[shift] ;

			if( table.get_value(my, op, y+shift) )
				return true ;
		}
		return false ;
	}
/*
	std::pair<int,int>* get_valid_move(std::pair<int,int>* val){
		for(int x = 0 ; x < 8 ; ++x){
			for(int y = 0 ; y < 8 ; ++y){
				if( is_valid_move(x, y) ){
					*val = std::pair<int,int>(x,y) ;
					++val ;
				}
			}
		}

		return val ;
	}
*/
	std::pair<int,int>* get_valid_move(std::pair<int,int>* val){
		static GetValidTable table = GetValidTable() ;

		unsigned long long my_board, op_board ;
		if( my_tile ){
			my_board = white ;
			op_board = black ;
		}
		else {
			my_board = black ;
			op_board = white ;
		}

		unsigned long long validpos = 0 ;

		static const unsigned long long row_mask = 	0xFF ;
		static const unsigned long long col_mask = 	0x0101010101010101 ;
		static const unsigned long long mplus_mask = 	0x0102040810204080 ;
		static const unsigned long long mminus_mask = 	0x8040201008040201 ;
		static const unsigned long long col_ones[] = {0, 0x0101010101010101, 
			0x0303030303030303, 0x0707070707070707, 0x0F0F0F0F0F0F0F0F,
			0x1F1F1F1F1F1F1F1F, 0x3F3F3F3F3F3F3F3F, 0x7F7F7F7F7F7F7F7F,
			0xFFFFFFFFFFFFFFFF } ;

		static const unsigned long long col_magic = mplus_mask ;
		static const unsigned long long m_magic = col_mask ;

		// Update row
		for(int x = 0 ; x < 8 ; ++x){
			int shift = x<<3 ;

			unsigned long long my = (my_board>>shift)&row_mask ;
			unsigned long long op = (op_board>>shift)&row_mask ;

			unsigned long long validline = table.get_value(0, my, op) ;
			validpos |= (validline<<shift) ;
		}

		// Update column
		for(int y = 0 ; y < 8 ; ++y){
			unsigned long long my = my_board>>y ;
			my = my & col_mask ;
			my = my * col_magic ;
			my = my>>56 ;

			unsigned long long op = op_board>>y ;
			op = op & col_mask ;
			op = op * col_magic ;
			op = op>>56 ;

			unsigned long long validline = table.get_value(1, my, op) ;
			validpos |= (validline<<y) ;
		}

		// Update slope where m > 0
		for(int x = 1 ; x < 8 ; ++x){
			int shift = x<<3 ;

			unsigned long long my = my_board>>shift ;
			my = my & mplus_mask ;
			my = my * m_magic ;
			my = my>>56 ;

			unsigned long long op = op_board>>shift ;
			op = op & mplus_mask ;
			op = op * m_magic ;
			op = op | col_ones[x] ;
			op = op>>56 ;

			unsigned long long validline = table.get_value(2, my, op) ;
			validpos |= (validline<<shift) ;
		}

		for(int y = 0 ; y < 8 ; ++y){
			unsigned long long my = my_board<<y ;
			my = my & mplus_mask ;
			my = my * m_magic ;
			my = my & ~col_ones[y] ;
			my = my>>56 ;

			unsigned long long op = op_board<<y ;
			op = op & mplus_mask ;
			op = op * m_magic ;
			op = op | col_ones[y] ;
			op = op>>56 ;

			unsigned long long validline = table.get_value(2, my, op) ;
			validpos |= (validline>>y) ;
		}

		// Check slope where m < 0
		for(int x = 1 ; x < 8 ; ++x){
			int shift = x<<3 ;
			
			unsigned long long my = my_board<<shift ;
			my = my & mminus_mask ;
			my = my * m_magic ;
			my = my>>56 ;

			unsigned long long op = op_board<<shift ;
			op = op & mminus_mask ;
			op = op * m_magic ;
			op = op>>56 ;

			unsigned long long validline = table.get_value(3, my, op) ;
			validpos |= (validline>>shift) ;
		}

		for(int y = 0 ; y < 8 ; ++y){
			unsigned long long my = my_board<<y ;
			my = my & mminus_mask ;
			my = my * m_magic ;
			my = my & ~col_ones[y] ;
			my = my>>56 ;

			unsigned long long op = op_board<<y ;
			op = op & mminus_mask ;
			op = op * m_magic ;
			op = op | col_ones[y] ;
			op = op>>56 ;

			unsigned long long validline = table.get_value(3, my, op) ;
			validpos |= (validline>>y) ;
		}

		// Put valid moves in pair array
		while( validpos ){
			int p = __builtin_ctzll(validpos) ;
			*val = std::pair<int,int>(p/8, p%8) ;
			++val ;
			validpos = validpos & ~(1ULL<<p) ;

		}

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

	inline int get_count()const{
		return __builtin_popcountll(black|white) ;
	}

	inline int get_my_score()const{
		return (my_tile? (__builtin_popcountll(white) - __builtin_popcountll(black)) : (__builtin_popcountll(black) - __builtin_popcount(white))) ;
	}

	inline int get_score()const{
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

	inline int get_pass()const{
		return pass;
	}

	inline bool get_my_tile()const{
		return my_tile;
	}

	inline bool is_game_over()const{
		return pass==2;
	}
};
#endif
