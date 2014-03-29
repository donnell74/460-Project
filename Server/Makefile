CC = g++
CFLAGS = --std=c++0x -Wall -I$(INC) 
INC = ../Includes

build: server.cpp $(INC)/cardlib.o $(INC)/networking.o 
	$(CC) $(CFLAGS) -pthread $(INC)/networking.o $(INC)/cardlib.o -o server server.cpp 


%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<
