#include<utility>

std::pair<unsigned long long, unsigned long long> mpos_mirror(unsigned long long black, unsigned long long white){
	const unsigned long long mask = 1 ;

	/* if i + j < 7
	 * 	new_i = i + (7-i-j) = 7 - j
	 * 	new_j = j + (7-i-j) = 7 - i 
	 * if i + j > 7
	 * 	new_i = i - (7-i-j) = i + i + j - 7 
	 * 	new_j = j - (7-i-j) = i + j + j - 7 */

	unsigned long long new_black = 0 ;
	for(int i = 0 ; i < 8 ; ++i)
		for(int j = 0 ; i + j <= 7 ; ++j)
			new_black |= (((black>>((i<<3)+j))&mask) << (((7-j)<<3)+(7-i))) ;
	for(int i = 0 ; i < 8 ; ++i)
		for(int j = 0 ; i + j <= 7 ; ++j)
			new_black |= (((black>>(((7-j)<<3)+(7-i)))&mask) << ((i<<3)+j)) ;

	unsigned long long new_white = 0 ;
	for(int i = 0 ; i < 8 ; ++i)
		for(int j = 0 ; i + j <= 7 ; ++j)
			new_white |= (((white>>((i<<3)+j))&mask) << (((7-j)<<3)+(7-i))) ;	
	for(int i = 0 ; i < 8 ; ++i)
		for(int j = 0 ; i + j <= 7 ; ++j)
			new_white |= (((white>>(((7-j)<<3)+(7-i)))&mask) << ((i<<3)+j)) ;

	return std::pair<unsigned long long, unsigned long long>(new_black, new_white) ;
}

std::pair<unsigned long long, unsigned long long> mneg_mirror(unsigned long long black, unsigned long long white){
	unsigned long long mask = 1 ;

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

std::pair<unsigned long long, unsigned long long> double_mirror(unsigned long long black, unsigned long long white){
	std::pair<unsigned long long, unsigned long long> b1 = mpos_mirror(black, white) ;
	std::pair<unsigned long long, unsigned long long> b2 = mneg_mirror(b1.first, b1.second) ;
	return b2 ;
}

/*
std::pair<int,int> OpenBook(unsigned long long black, unsigned long long white){
	int count =  __builtin_popcountll(black|white) ;

	switch(count){
		case 4:
			return std::pair<int,int>(3,2) ;
		case 5:
			return std::pair<int,int>(8,0) ;
			break ;
		case 6:
			return std::pair<int,int>(8,0) ;
			break ;
		default:
			return std::pair<int,int>(8,0) ;
	}
}

*/
