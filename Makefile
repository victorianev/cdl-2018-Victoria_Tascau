run: code.o
	./code.o

build: code.cpp
	g++ -std=c++11 -o code.o code.cpp

clean: code.o
	rm -f ./code.o
