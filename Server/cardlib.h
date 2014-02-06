#include<iostream>
#include<vector>
#include<random>
#ifndef CARDLIB_H
#define CARDLIB_H
#define MAX_DECK_SIZE 81
#define CARD_REF deck->cards[i]

using namespace std;

// DO NOT CHANGE TO 0
// change to 0 will cause null byte
// to be sent if all 0s
enum Number
{
  ONE = 1,
  TWO,
  THREE
};

enum Symbol
{
  CROSS = 1,
  SQUARE,
  SLASH
};

enum Shade
{
  CIRCLE = 1,
  AT,
  SPACE
};

enum Color
{
  RED = 1,
  PURPLE,
  GREEN
};

/* Card structure
Each card has a number, represented
as an int, a symbol, shade, color, 
each represented as strings and a bitcode
to ID each card 
*/
struct Card
{
  enum Number number;
  enum Symbol symbol;
  enum Shade shade;
  enum Color color;
  char bitcode;
};

/*Deck structure to hold card structs
Deck contains a vector of card structs
and a count of the current cards in 
the deck*/

struct Deck
{
  int card_count;
  vector<Card*>cards;
};


Deck* deck_init();

//Shuffles the deck using Fisher-Yates Shuffle
void shuffle_deck( Deck* deck );

//Displays the current contents of a deck struct
void display_deck( Deck* deck );

//Draw function to draw a card from the deck
Card* draw( Deck* deck );

//Display memory addresses for cards in the deck
void memory_addresses( Deck* deck );

#endif /* CARDLIB_H */
