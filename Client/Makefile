CC = g++
CFLAGS = --std=c++0x -Wall -I$(INC)
INC = ../Includes
LDLIBS = -lcurses

build: client.cpp $(INC)/networking.o
	$(CC) $(CFLAGS) $(INC)/networking.o -o client client.cpp $(LDLIBS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<
