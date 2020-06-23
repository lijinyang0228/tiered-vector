all: example

example: example.cpp  HeaderFiles/tiered-vector.h
		g++ -std=c++11 example.cpp -o example
