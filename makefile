all: search.cpp OTP.h board.h
	g++ search.cpp -std=c++11 -march=native -funroll-loops -Wall -O3 -o Othello

