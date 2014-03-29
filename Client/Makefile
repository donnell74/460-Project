CC = g++
CFLAGS = --std=c++0x -Wall -I$(INC)
INC = ../Includes
LDLIBS = -lcurses

<<<<<<< HEAD
build: client.cpp $(INC)/networking.o
	$(CC) $(CFLAGS) $(INC)/networking.o -o client client.cpp $(LDLIBS)
=======
build: client.cpp $(INC)/cardlib.o $(INC)/networking.o
	$(CC) $(CFLAGS) $(INC)/cardlib.o $(INC)/networking.o -o client client.cpp
>>>>>>> 0499b1edba32f4069a3e434ac4fb52542f4ed35f

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<
