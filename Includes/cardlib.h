#include<iostream>
#include<vector>
#include<string>
#include<random>
#ifndef CARDLIB_H
#define CARDLIB_H
#define MAX_DECK_SIZE 81

using namespace std;

static vector<int>std_indexes = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
static vector<vector<char>> ACCEPTED_CHARS = {
  { 'A', 'B', 'C', 'D', 
    'E', 'F', 'G', 'H',
    'I', 'J', 'K', 'L', 
    'X' },

  { '1', '2', '3', '4', 
    'Q', 'W', 'E', 'R',
    'A', 'S', 'D', 'F', 
    'N' },

  { '7', '8', '9', '0', 
    'U', 'I', 'O', 'P',
    'J', 'K', 'L', ';', 
    'N' },

  { '1', '2', '3', '4', 
    'Q', 'W', 'F', 'P',
    'A', 'R', 'S', 'T', 
    'N' },
};


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

//Set structure
//Structure to represent a triple of 
//indexes that represent a set in a 
//playing card map
struct Set
{
  int x;
  int y;
  int z;
};

/*Deck structure to hold card structs
Deck contains a vector of card structs
and a count of the current cards in 
the deck

struct Deck
{
  int card_count;
  vector<Card*>cards;
};
*/

/*
Deck* deck_init();

//Shuffles the deck using Fisher-Yates Shuffle
void shuffle_deck( Deck* deck );

//Displays the current contents of a deck struct
void display_deck( Deck* deck );

//Draw function to draw a card from the deck
Card* draw( Deck* deck );

//Display memory addresses for cards in the deck
void memory_addresses( Deck* deck );
*/

//Auxillary function to build set for set test
void set_auxillary();

//Tests a vector of three card pointers to see if the cards
//make up a valid set
bool check_set( vector<Card*> set );

//Comparison helper functions for check_set
bool color_same( vector<Card*> set );
bool symbol_same( vector<Card*> set );
bool shade_same( vector<Card*> set );
bool number_same( vector<Card*> set );
bool color_differs( vector<Card*> set );
bool symbol_differs( vector<Card*> set );
bool shade_differs( vector<Card*> set );
bool number_differs( vector<Card*> set );


/*Deck class
Member functions:
-Deck() - Deck initializer
-~Deck() - Deck deconstructor
-bool empty() - returns true if cards is empty
-int count() - returns count of cards in deck
-void display() - displays current cards in the deck
-void shuffle() - shuffles cards in deck
-void mem_display() - displays addresses of cards and deck in memory
-Card* draw() - draws the first card from the deck
-Card* get_card(int) - returns the card in the deck at the specified
index
-vector<Card*>get_cards() - returns all the cards in the deck
-void remove_card(int) - removes the card pointer at an index in the deck
-void add_card(Card*) - pushes card pointer to end of deck
Fields:
Private:
-vector<Card*>cards - vector of card structs the deck contains
-int top - index for a virtual top
-int count - keeps count of cards in deck
*/

class Deck
{
 private:
  vector < Card* > cards;
  int top;
  int _count;
   
 public:
  Deck();
  Deck( int code );
  ~Deck();
  bool empty( int code );
  vector <Card*>get_cards();
  int count( int code );
  int get_top(){return top;}
  void reset_top();
  void mem_display();
  void display( int code );
  void shuffle();
  void sort_cards();
  Card* draw();
  Card* get_card( int index );
  void remove_card( int index );
  void replace_card( int index, Card* ncard );
  void clear_cards();
  void add_card( Card* ncard );
  void remove_all_cards();
};


//Creates and returns a string of bitcodes from a deck pointer
//and adds the cards to a playing deck
string create_playing_cards( vector<int>indexes, Deck* deck, Deck* playing_deck );

int rand ( int );

//Displays information about a given card
void display_card( Card* ncard );

//Returns a vectpr of possible sets from a given set of cards
vector<Set*> find_sets ( vector<Card*>cards );

//Returns number of possible sets in a deck and fills the set field 
int num_sets ( vector<Card*>cards );

//Display the sets returned from find_sets
void display_sets ( vector<Card*>cards );

//Maps cards to index of playing card deck
//Returns index on success and -1 on error
int map_card ( char key, vector<char> accepted_chars );

//Checks client guess
//Returns indexes of needed cards in the playing deck 
vector<int> check_guess ( char* guess, Deck* deck, Deck* playing_deck );

#endif /* CARDLIB_H */


