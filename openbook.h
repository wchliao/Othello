#include<utility>
#include<cstdio>

std::pair<unsigned long long, unsigned long long> board_mpos_mirror(unsigned long long black, unsigned long long white){
	const unsigned long long mask = 1 ;

	unsigned long long new_black = 0 ;
	for(int i = 0 ; i < 8 ; ++i)
		for(int j = 0 ; j < 8 ; ++j)
			new_black |= (((black>>((i<<3)+j))&mask) << (((7-j)<<3)+(7-i))) ;

	unsigned long long new_white = 0 ;
	for(int i = 0 ; i < 8 ; ++i)
		for(int j = 0 ; j < 8 ; ++j)
			new_white |= (((white>>((i<<3)+j))&mask) << (((7-j)<<3)+(7-i))) ;	

	return std::pair<unsigned long long, unsigned long long>(new_black, new_white) ;
}

std::pair<unsigned long long, unsigned long long> board_mneg_mirror(unsigned long long black, unsigned long long white){
	const unsigned long long mask = 1 ;

	unsigned long long new_black = 0 ;
	for(int i = 0 ; i < 8 ; ++i)
		for(int j = 0 ; j < 8 ; ++j)
			new_black |= (((black>>((i<<3)+j))&mask) << ((j<<3)+i)) ;
	
	unsigned long long new_white = 0 ;
	for(int i = 0 ; i < 8 ; ++i)
		for(int j = 0 ; j < 8 ; ++j)
			new_white |= (((white>>((i<<3)+j))&mask) << ((j<<3)+i)) ;

	return std::pair<unsigned long long, unsigned long long>(new_black, new_white) ;
}

std::pair<unsigned long long, unsigned long long> board_double_mirror(unsigned long long black, unsigned long long white){
	std::pair<unsigned long long, unsigned long long> b1 = board_mpos_mirror(black, white) ;
	std::pair<unsigned long long, unsigned long long> b2 = board_mneg_mirror(b1.first, b1.second) ;
	return b2 ;
}

std::pair<int,int> pos_mpos_mirror(std::pair<int,int> pos){
	return std::pair<int,int>(7-pos.second, 7-pos.first) ;
}

std::pair<int,int> pos_mneg_mirror(std::pair<int,int> pos){
	return std::pair<int,int>(pos.second, pos.first) ;
}

std::pair<int,int> pos_double_mirror(std::pair<int,int> pos){
	return pos_mneg_mirror(pos_mpos_mirror(pos)) ;
}

std::pair<int,int> book5(unsigned long long black, unsigned long long white){
	if( black == 0x81C000000 && white == 0x1000000000 )
		return std::pair<int,int>(2,2) ;
	else
		return std::pair<int,int>(8,0) ;
}

std::pair<int,int> book6(unsigned long long black, unsigned long long white){
	if( black == 0x814000000 && white == 0x1008040000 )
		return std::pair<int,int>(2,3) ;
	else if( black == 0x80C000000 && white == 0x1010100000 )
		return std::pair<int,int>(5,5) ;
	else if( black == 0x1C000000 && white == 0x1C00000000 )
		return std::pair<int,int>(5,3) ;
	else
		return std::pair<int,int>(8,0) ;
}

std::pair<int,int> book7(unsigned long long black, unsigned long long white){
	if( black == 0x81C080000 && white == 0x1000040000 )
		return std::pair<int,int>(4,2) ;
	else if( black == 0x380C000000 && white == 0x10100000 )
		return std::pair<int,int>(3,1) ;
	else if( black == 0x30180C000000 && white == 0x10100000 )
		return std::pair<int,int>(5,4) ;
	else
		return std::pair<int,int>(8,0) ;
}

std::pair<int,int> book8(unsigned long long black, unsigned long long white){
	if( black == 0x18080000 && white == 0x1C04040000 )
		return std::pair<int,int>(5,3) ;
	else if( black == 0x30080C000000 && white == 0x101010100000 )
		return std::pair<int,int>(4,5) ;
	else if( black == 0x201800000000 && white == 0x1E100000 )
		return std::pair<int,int>(2,5) ;
	else if( black == 0x3800000000 && white == 0x1E100000 )
		return std::pair<int,int>(2,5) ;
	else
		return std::pair<int,int>(8,0) ;
}

std::pair<int,int> book9(unsigned long long black, unsigned long long white){
	if( black == 0x1E080000 && white == 0x1C00040000 )
		return std::pair<int,int>(1,3) ;
	else if( black == 0x80818080000 && white == 0x1404040000 )
		return std::pair<int,int>(2,4) ; 
	else if( black == 0x3810300000 && white == 0xE100000)
		return std::pair<int,int>(3,5) ;
	else if( black == 0x20380C000000 && white == 0x100010100000 )
		return std::pair<int,int>(4,2) ;
	else
		return std::pair<int,int>(8,0) ;
}

std::pair<int,int> book10(unsigned long long black, unsigned long long white){
	if( black == 0x16000000 && white == 0x1C080C0800 )
		return std::pair<int,int>(5,3) ;
	else if( black == 0x6000000 && white == 0x1C181C0000 )
		return std::pair<int,int>(1,4) ;
	else if( black == 0x80800080000 && white == 0x143C040000 )
		return std::pair<int,int>(4,5) ;
	else if( black == 0x80800000000 && white == 0x141C1C0000 )
		return std::pair<int,int>(4,5) ;
	else if( black == 0x180C000000 && white == 0x702010100000 )
		return std::pair<int,int>(2,5) ;
	else if( black == 0x203804000000 && white == 0x100418100000 )
		return std::pair<int,int>(3,5) ;
	else
		return std::pair<int,int>(8,0) ;
}

std::pair<int,int> book11(unsigned long long black, unsigned long long white){
	if( black == 0x80416000000 && white == 0x18080C0800)
		return std::pair<int,int>(2,1) ;
	else if( black == 0x16081000 && white == 0x1C08040800 )
		return std::pair<int,int>(2,1) ;
	else if( black == 0x83810080000 && white == 0x42C040000 )
		return std::pair<int,int>(1,3) ;
	else if( black == 0x80C02080000 && white == 0x103C040000 )
		return std::pair<int,int>(5,1) ;
	else if( black == 0x20383C000000 && white == 0x100400100000 )
		return std::pair<int,int>(5,6) ;
	else
		return std::pair<int,int>(8,0) ;
}

std::pair<int,int> book12(unsigned long long black, unsigned long long white){
	if( black == 0x83810000000 && white == 0x42C0C0800 )
		return std::pair<int,int>(2,5) ;
	else if( black == 0x383C000000 && white == 0x700400100000  ) 
		return std::pair<int,int>(6,5) ;
	else
		return std::pair<int,int>(8,0) ;
}

std::pair<int,int> book13(unsigned long long black, unsigned long long white){
	if( black == 0x83E10000000 && white == 0x2C0C0800 )
		return std::pair<int,int>(5,2) ;
	else if( black == 0x2030383C000000 && white == 0x400400100000 )
		return std::pair<int,int>(4,6) ;
	else
		return std::pair<int,int>(8,0) ;
}

std::pair<int,int> book14(unsigned long long black, unsigned long long white){
	if( black == 0x2030182C000000 && white == 0x402410180000)
		return std::pair<int,int>(2,5) ;
	else if( black == 0x2000383C000000 && white == 0x780400100000 )
		return std::pair<int,int>(6,4) ;
	else if( black == 0x2030001C000000 && white == 0x407C20100000 )
		return std::pair<int,int>(5,3) ;
	else
		return std::pair<int,int>(8,0) ;
}

std::pair<int,int> SearchBook(std::pair<int,int> (*func)(unsigned long long, unsigned long long), unsigned long long black, unsigned long long white){
	std::pair<int,int> pos ;
	std::pair<unsigned long long, unsigned long long> board ;
	
	pos = func(black, white) ;
	if( pos != std::pair<int,int>(8,0) ){
		fprintf(stderr, "Open Book Hit Type 1.\n") ;
		return pos ;
	}

	board = board_mneg_mirror(black, white) ;
	pos = func(board.first, board.second) ;
	if( pos != std::pair<int,int>(8,0) ){
		fprintf(stderr, "Open Book Hit Type 2.\n") ;
		return pos_mneg_mirror(pos) ;
	}

	board = board_mpos_mirror(black, white) ;
	pos = func(board.first, board.second) ;
	if( pos != std::pair<int,int>(8,0) ){
		fprintf(stderr, "Open Book Hit Type 3.\n") ;
		return pos_mpos_mirror(pos) ;
	}

	board = board_mneg_mirror(board.first, board.second) ;
	pos = func(board.first, board.second) ;
	if( pos != std::pair<int,int>(8,0) ){
		fprintf(stderr, "Open Book Hit Type 4.\n") ;
		return pos_double_mirror(pos) ;
	}

	fprintf(stderr, "Open Book No Hit.\n") ;
	return std::pair<int,int>(8,0) ;
}

// Public function for call
std::pair<int,int> OpenBook(unsigned long long black, unsigned long long white){
	int count =  __builtin_popcountll(black|white) ;

	switch(count){
		case 4:
			return std::pair<int,int>(3,2) ;
		case 5: 
			return SearchBook(&book5, black, white) ;
		case 6: 
			return SearchBook(&book6, black, white) ;
		case 7: 
			return SearchBook(&book7, black, white) ;
		case 8: 
			return SearchBook(&book8, black, white) ;
		case 9: 
			return SearchBook(&book9, black, white) ;
		case 10: 
			return SearchBook(&book10, black, white) ;
		case 11: 
			return SearchBook(&book11, black, white) ;
		case 12: 
			return SearchBook(&book12, black, white) ;
		case 13: 
			return SearchBook(&book13, black, white) ;
		case 14: 
			return SearchBook(&book14, black, white) ;
		default:
			return std::pair<int,int>(8,0) ;
	}
}
