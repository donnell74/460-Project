CC = g++
CFLAGS = --std=c++0x -I $(INC)
INC = ../Includes
LDLIBS = -lcurses -pthread

build: play.cpp $(INC)/cardlib.o $(INC)/client.o
	$(CC) $(CFLAGS) $(INC)/cardlib.o $(INC)/client.o -o play play.cpp $(LDLIBS)
%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<
