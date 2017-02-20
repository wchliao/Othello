all: search.cpp OTP.h board.h
	g++ search.cpp -D PP -std=c++11 -march=native -Wall -O3 -o B02902105

UCB: search.cpp OTP.h board.h 
	g++ search.cpp -D UCB -std=c++11 -march=native -Wall -O3 -o B02902105

UCT: search.cpp OTP.h board.h 
	g++ search.cpp -D UCT -std=c++11 -march=native -Wall -O3 -o B02902105

PP: search.cpp OTP.h board.h 
	g++ search.cpp -D PP -std=c++11 -march=native -Wall -O3 -o B02902105

