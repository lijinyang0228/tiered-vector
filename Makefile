all: example

example: example.cpp  HeaderFiles/templated_tiered.h
		g++ -std=c++11 example.cpp -o example
