#include "cardlib.h"
//Initializer for Deck struct. Returns a deck struct pointer
Deck* deck_init()
{
  Deck* ndeck = new Deck;
  /* Not Used but may be useful later
  string  shade[3];
  string symbols[3];
  string colors[3];

  shade[0] = "solid";
  shade[1] = "striped";
  shade[2] = "open";

  symbols[0] = "symbol_1";
  symbols[1] = "symbol_2";
  symbols[2] = "symbol_3";

  colors[0] = "red";
  colors[1] = "green";
  colors [2] = "blue";
  */

  for ( int i=0; i<3; i++ )
  {
    for ( int j=0; j<3; j++ )
    {
      for ( int k=0; k<3; k++ )
      {
        for ( int l=0; l<3; l++ ) 
        {
          Card* ncard = new Card;
          ncard->number = static_cast<Number>( i );
          ncard->symbol = static_cast<Symbol>( j );
          ncard->shade = static_cast<Shade>( k );
          ncard->color = static_cast<Color>( l );
          ncard->bitcode = ( i << 6 ) | ( j << 4 ) | ( k << 2 ) | l;
          ndeck->cards.push_back( ncard );
        }
      }
    }
  }
  
  ndeck->card_count = ndeck->cards.size();

  return ndeck;
}

//Shuffles the deck using Fisher-Yates Shuffle
void shuffle_deck( Deck* deck )
{
   random_device rd;
  default_random_engine e1( rd() );
  
  //Pointer to Card struct for temporary storage
  Card* temp;
  for ( int i=deck->card_count-1; i>0; i-- )
  {
    uniform_int_distribution<int> uniform_dist( 0,i );
    int random_number = uniform_dist( e1 );

    temp = CARD_REF;
    CARD_REF = deck->cards[random_number];
    deck->cards[random_number] = temp;
  }

}

//Displays the current contents of a deck struct
void display_deck( Deck* deck )
{
  if ( deck == nullptr )
  {
    cout<<"Invalid deck pointer"<<endl;
    exit( EXIT_FAILURE );
  }

  for ( int i=0; i<deck->card_count; i++ )
  {
    cout<<"Deck["<<i+1<<"]:"<<CARD_REF->number<<" "<<CARD_REF->symbol<<" "<<CARD_REF->shade<<" "<<CARD_REF->color
        << " " << CARD_REF->bitcode << endl;
  }

  cout<<"Card Count:"<<deck->card_count<<endl;
  cout<<"x----------------------------------------------x"<<endl;
}


void memory_addresses( Deck* deck )
{
  //Deck Memory Address
  cout<<"Deck Memory Address:"<<&deck<<endl;
  for ( int i=0; i<deck->card_count; i++ )
  {
    cout<<"Deck["<<i+1<<"]"<<deck->cards[i]<<endl;
  }
}

//Draws a card from the top of the deck
Card* draw( Deck* deck )
{

  if ( deck == nullptr )
    {
      cout<<"Invalid deck pointer passed to draw function"<<endl;
      exit( EXIT_FAILURE );
    }

  if ( deck->card_count == 0 )
    {
      cout<<"Deck is empty"<<endl;
      return nullptr;
    }

    Card* ncard = deck->cards[0];

    deck->cards.erase( deck->cards.begin() );
    deck->card_count -= 1;
    
    return ncard;
}
