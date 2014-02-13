#include<iostream>
#include<vector>
#include<random>
#ifndef CARDLIB_H
#define CARDLIB_H
#define MAX_DECK_SIZE 81
#define CARD_REF deck->cards[i]

using namespace std;

/*Card structure
Each card has a number, represented
as an int, a symbol, shade, color, 
each represented as strings and a bitcode
to ID each card 
*/

struct Card{
  int number;
  string symbol;
  string shade;
  string color;
  int bitcode;
};

/*Deck structure to hold card structs
Deck contains a vector of card structs
and a count of the current cards in 
the deck*/

struct Deck{
  int card_count;
  vector<Card*>cards;
};


Deck* deck_init();
void shuffleDeck(Deck* deck);

#endif /* CARDLIB_H */
