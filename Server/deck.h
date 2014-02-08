#include<vector>
#include "cardlib.h"
#ifndef DECK_H
#define DECK_H

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
  ~Deck();
  bool empty( ){ return cards.empty(); }
  int count( ){ return _count; }
  void mem_display();
  void display();
  void shuffle();
  Card* draw();
  void add_card( Card* ncard );
};


#endif /* DECK_H */

