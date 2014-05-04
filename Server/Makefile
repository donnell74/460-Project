CC = g++
CFLAGS = --std=c++0x -Wall -I$(INC) 
INC = ../Includes

build: game.cpp $(INC)/cardlib.o $(INC)/server.o 
	$(CC) $(CFLAGS) -pthread  -g $(INC)/server.o $(INC)/cardlib.o -o game game.cpp 


%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<
