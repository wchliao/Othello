#include<utility>
		
class UpdateTable {

	std::pair<unsigned long long, unsigned long long> table[2097152] ;

	public:

	UpdateTable(){
		// Row's look-up table
		for(int my = 0 ; my < 256 ; ++my){
			for(int op = 0 ; op < 256 ; ++op){
				unsigned long long pos = 1 ;
				for(int p = 0 ; p < 8 ; ++p, pos = pos<<1){

					table[get_index(0, my, op, p)] = std::pair<unsigned long long,unsigned long long>(my|pos,op&~pos) ;

					if( (my&op) || (my&pos) || (op&pos) )
						continue ;

					// right
					unsigned long long tmppos = pos<<1 ;
					unsigned char y = pos<<1 ;
					unsigned long long tmp_my = get_value(0, my, op, p).first ;
					unsigned long long tmp_op = get_value(0, my, op, p).second ;
					while( op & tmppos & y ){
						tmp_my |= tmppos ;
						tmp_op &= ~tmppos ;
						tmppos = tmppos<<1 ;
						y = y<<1 ;
					}
					if( my & tmppos & y ){
						table[get_index(0, my, op, p)].first = tmp_my ;
						table[get_index(0, my, op, p)].second = tmp_op ;
					}
		
					// left
					tmppos = pos>>1 ;
					y = pos>>1 ;
					tmp_my = get_value(0, my, op, p).first ;
					tmp_op = get_value(0, my, op, p).second ;
					while( op & tmppos & y ){
						tmp_my |= tmppos ;
						tmp_op &= ~tmppos ;
						tmppos = tmppos>>1 ;
						y = y>>1 ;
					}
					if( my & tmppos & y ){
						table[get_index(0, my, op, p)].first = tmp_my ;
						table[get_index(0, my, op, p)].second = tmp_op ;
					}
				}
			}
		}

		// Column's look-up table
		for(int my = 0 ; my < 256 ; ++my){
			for(int op = 0 ; op < 256 ; ++op){
				for(int p = 0 ; p < 8 ; ++p){
					unsigned long long row_my = get_value(0, my, op, p).first ;
					unsigned long long row_op = get_value(0, my, op, p).second ;
					unsigned long long tmp_my = 0 ;
					unsigned long long tmp_op = 0 ;
					const unsigned long long mask = 1 ;

					for(int y = 0 ; y < 8 ; y++){
						tmp_my |= (((row_my>>y)&mask)<<(y<<3)) ;
						tmp_op |= (((row_op>>y)&mask)<<(y<<3)) ;
					}

					table[get_index(1, my, op, p)] = std::pair<unsigned long long,unsigned long long>(tmp_my,tmp_op) ;
				}
			}
		}		

		// Mpuls's look-up table
		for(int my = 0 ; my < 256 ; ++my){
			for(int op = 0 ; op < 256 ; ++op){
				for(int p = 0 ; p < 8 ; ++p){
					unsigned long long row_my = get_value(0, my, op, p).first ;
					unsigned long long row_op = get_value(0, my, op, p).second ;
					unsigned long long tmp_my = 0 ;
					unsigned long long tmp_op = 0 ;
					const unsigned long long mask = 1 ;

					for(int y = 0 ; y < 8 ; y++){
						tmp_my |= (((row_my>>y)&mask)<<(((7-y)<<3)+y)) ;
						tmp_op |= (((row_op>>y)&mask)<<(((7-y)<<3)+y)) ;
					}

					table[get_index(2, my, op, p)] = std::pair<unsigned long long,unsigned long long>(tmp_my,tmp_op) ;
				}
			}
		}		

		// Mminus's look-up table
		for(int my = 0 ; my < 256 ; ++my){
			for(int op = 0 ; op < 256 ; ++op){
				for(int p = 0 ; p < 8 ; ++p){
					unsigned long long row_my = get_value(0, my, op, p).first ;
					unsigned long long row_op = get_value(0, my, op, p).second ;
					unsigned long long tmp_my = 0 ;
					unsigned long long tmp_op = 0 ;
					const unsigned long long mask = 1 ;

					for(int y = 0 ; y < 8 ; y++){
						tmp_my |= (((row_my>>y)&mask)<<(y*9)) ;
						tmp_op |= (((row_op>>y)&mask)<<(y*9)) ;
					}

					table[get_index(3, my, op, p)] = std::pair<unsigned long long,unsigned long long>(tmp_my,tmp_op) ;
				}
			}
		}		
	}
	
	inline int get_index(int a, int b, int c, int d){
		return ((a<<19) | (b<<11) | (c<<3) | d) ; /* a*256*256*8 + b*256*8 + c*8 + d */
	}

	inline std::pair<unsigned long long, unsigned long long> get_value(int a, int b, int c, int d){
		return table[(a<<19) | (b<<11) | (c<<3) | d] ;
	}
	
} ;

class IsValidTable {

	bool table[524288] ;

	public:

	IsValidTable(){
		for(int my = 0 ; my < 256 ; ++my){
			for(int op = 0 ; op < 256 ; ++op){
				unsigned char pos = 1 ;
				for(int p = 0 ; p < 8 ; ++p, pos = pos<<1){

					table[get_index(my, op, p)] = false ;

					if( (my&op) || (my&pos) || (op&pos) )
						continue ;

					// right
					unsigned char tmppos = pos<<1 ;
					if( op & tmppos ){
						do {
							tmppos = tmppos<<1 ;
						}while( op & tmppos ) ;
						if( my & tmppos ){
							table[get_index(my, op, p)] = true ;
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
							table[get_index(my, op, p)] = true ;
							continue ;
						}
					}
				}
			}
		}
	}

	inline int get_index(int b, int c, int d){
		return ((b<<11) | (c<<3) | d) ; /* b*256*8 + c*8 + d */
	}

	inline bool get_value(int b, int c, int d){
		return table[(b<<11) | (c<<3) | d] ;
	}
} ;

/*	unsigned char*** construct_get_valid_move_table(){
		std::pair<unsigned long long, unsigned long long>**** table = new std::pair<unsigned long long,unsigned long long>***[4] ;
		for(int i = 0 ; i < 4 ; i++){
			table[i] = new std::pair<unsigned long long,unsigned long long>**[256] ;
			for(int j = 0 ; j < 256 ; j++){
				table[i][j] = new std::pair<unsigned long long,unsigned long long>*[256] ;
				for(int k = 0 ; k < 256 ; k++)
					table[i][j][k] = new std::pair<unsigned long long,unsigned long long>[8] ;
			}
		}
	}
*/

