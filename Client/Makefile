CC = g++
CFLAGS = --std=c++0x -Wall -I$(INC)
INC = ../Includes

build: client.cpp $(INC)/cardlib.o $(INC)/networking.o
	$(CC) $(CFLAGS) $(INC)/cardlib.o $(INC)/networking.o -o client client.cpp

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<
