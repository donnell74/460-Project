CC = g++
CFLAGS = --std=c++0x -Wall -I$(INC) 
INC = ../Includes

build: server.cpp cardlib.o $(INC)/networking.o 
	$(CC) $(CFLAGS) -pthread $(INC)/networking.o cardlib.o -o server server.cpp 


%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<
