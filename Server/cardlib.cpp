#include "cardlib.h"
#include <vector>
#include <iostream>
using namespace std;

void display_card(Card* ncard)
{
  cout<<" Number:" << ncard->number << " Symbol:" << ncard->symbol << " Shade:" << ncard->shade << " Color:" << ncard->color << "Bitcode:" << ncard->bitcode << endl;
}


Deck::Deck()
{
  //Initiliazing deck object
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
          cards.push_back( ncard );
        }
      }
    }
  }

  top = 0;
  _count = cards.size();
}


Deck::Deck( int code )
{
  //Construct deck object
}


Deck::~Deck()
{
  //Deconstructed deck object
}


int Deck::count( int code )
{
  switch( code )
    {
    case 0:
      return _count;

    case 1:
      return cards.size();

    default:
      break;
    } 

  return cards.size(); 
}

Card* Deck::get_card( int index )
{
  return cards[index];
}


vector<Card*> Deck::get_cards()
{
  return cards;
}


void Deck::display( int code )
{
  if( !empty( code ) )

    {

  int limit, initial_count;

  if ( code == 0 )
    {

      initial_count = top;
      limit = top + _count;
    }

  else
    {
      limit = cards.size();
      initial_count = 0;
  }

  int j = 0;
  for ( int i=initial_count; i<limit; i++ )
  {
    cout << "Deck[" << j << "] Number:" << cards[i]->number << " Symbol:" << cards[i]->symbol << " Shade:" << cards[i]->shade << " Color:" << cards[i]->color << "Bitcode:" << cards[i]->bitcode << endl;
    j++;
  }

  cout << "Card Count:" << count( code ) << endl;
  cout << "x----------------------------------------------x"<<endl;
  cout<<"top:"<<top<<endl;
  cout<<"virtual count:"<<_count<<endl;

    }
  else
    {
      cout << "Deck is empty" << endl;
    }
}


void Deck::shuffle()
{
  if( !empty( 0 ) )
    {
      random_device rd;
      default_random_engine e1( rd() );
  
      //Pointer to Card struct for temporary storage
      Card* temp;
      for ( int i = _count -1; i>top; i-- )
	{
	  uniform_int_distribution<int> uniform_dist( 0,i );
	  int random_number = uniform_dist( e1 );

	  temp = cards[i];
	  cards[i] = cards[random_number];
	  cards[random_number] = temp;
	}
    }

  else
    {
      cout << "Deck is empty" <<endl;
    }

}


void Deck::mem_display( )
{
  //Deck Memory Address
  for ( int i=top; i<_count; i++ )
  {
    cout<<"Deck["<<i+1<<"]"<<cards[i]<<endl;
  }
}


Card* Deck::draw()
{
  Card* ncard = cards[top];
  top+=1;
  _count-=1;
  return ncard;
}

void Deck::remove_card( int index )
{
  //Should only be called on playing deck
  cards.erase( cards.begin() + index );
  cout << "Removed card at index " << index << endl;
}

void Deck::replace_card( int index, Card* ncard )
{
  //Replaces card in index with new card pointer
  cout << "Replacing ";
  display_card( cards[index] );
  cout << "With:";
  display_card( ncard );

  cards[index] = ncard;
}

void Deck::clear_cards()
{
  cards.clear();
}

void Deck::add_card( Card* ncard )
{
  cards.push_back( ncard );
}


bool Deck::empty( int code )
{
  switch( code )
    {
    case 0:
      if( top == 80)
	{
	  return true;
	}
      break;
    case 1:
      return cards.empty();
 
    default:
      break;
    }
  return false;
}



/*
//Initializer for Deck struct. Returns a deck struct pointer
Deck* deck_init()
{
  Deck* ndeck = new Deck;
   Not Used but may be useful later
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
*/
/*
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
*/

//Boolean comparison card set functions
//Difference comparisons
bool color_differs( vector<Card*>set )
{
  //Returns false if all colors are the same
  return set[0]->color != set[1]->color && set[0]->color != set[2]->color && set[1]->color != set[2]->color; 
}


bool symbol_differs( vector<Card*>set )
{
  //Returns false if all symbols are the same
  return set[0]->symbol != set[1]->symbol && set[0]->symbol != set[2]->symbol && set[1]->symbol != set[2]->symbol; 
}


bool shade_differs( vector<Card*>set )
{
  //Returns false if all shades are the same
  return set[0]->shade != set[1]->shade && set[0]->shade != set[2]->shade && set[1]->shade != set[2]->shade;
}


bool number_differs( vector<Card*>set )
{
  //Returns false if at least two numbers are the same
  return set[0]->number != set[1]->number && set[0]->number != set[2]->number && set[1]->number != set[2]->number;
}


//Same comparisons
bool color_same( vector<Card*>set )
{
  //Returns false if all colors are not the same
  return set[0]->color == set[1]->color && set[0]->color == set[2]->color && set[1]->color == set[2]->color;
}

bool symbol_same( vector<Card*>set )
{
  //Returns false if all symbols aren't the same
  return set[0]->symbol == set[1]->symbol && set[0]->symbol == set[2]->symbol && set[1]->symbol == set[2]->symbol;
}

bool shade_same( vector<Card*>set )
{
  //Returns false if all shades arent the same
  return set[0]->shade == set[1]->shade && set[0]->shade == set[2]->shade && set[1]->shade == set[2]->shade;
}

bool number_same( vector<Card*>set )
{
  //Returns false if all numbers arent the same
  return set[0]->number == set[1]->number && set[0]->number == set[2]->number && set[1]->number == set[2]->number;
}

//Checks if array of sent cards is set
bool check_set ( vector<Card*>set )
{
  /*Logic - set must meet all conditions to be a valid set
  They all have the same number, or they have three different numbers.
  They all have the same symbol, or they have three different symbols.
  They all have the same shading, or they have three different shadings.
  They all have the same color, or they have three different colors.*/
  
  if( ( number_same( set ) || number_differs( set ) ) && ( symbol_same( set ) || symbol_differs( set ) ) && ( shade_same( set ) || shade_differs( set ) ) && ( color_same( set ) || color_differs( set ) ) )
    {
     return true;
    }
    
  else{
    return false;
  }
  
}

void set_auxillary ( )
{
  string attr[] = {"color", "symbol", "shade", "number"};
  string prompts[] = {"Color:  red - 0\n\tpurple - 1\n\tgreen - 2","symbol: cross - 0\n\tsquare - 1\n\tslash - 2", "shade:  circle - 0\n\tat - 1\n\tspace - 2","Number: one - 0\n\ttwo - 1\n\tthree - 2"};
 

  vector<Card*>set;
  int mcards = 3;
  
  for( int j=0; j<mcards; j++ )
    {
      int ins[4];
      Card* ncard = new Card;
      for ( int i=0; i<4; i++ )
	{
	  cout << prompts[i] << endl;
	  cout << "Card "<< j+1 << " " << attr[i] << ":" << endl;
	  cin >> ins[i];
	}
      ncard->color = static_cast<Color>( ins[0] );
      ncard->symbol = static_cast<Symbol>( ins[1] );
      ncard->shade = static_cast<Shade>( ins[2] );
      ncard->number = static_cast<Number>( ins[3] );
      set.push_back( ncard );
      cout << "card_color:" << set[j]->color << endl;
      cout << "card_symbol:" << set[j]->symbol << endl;
      cout << "card_shade:" << set[j]->shade << endl;
      cout << "card_number:" << set[j]->number << endl;

    }

      

  if( check_set( set ) )
    {
      cout << "These cards form a valid set" << endl;
    }

  else
    {
      cout << "Cards do not form a valid set" << endl;
    }
}

string create_playing_cards( int cards_needed, Deck* deck, Deck* playing_deck)
{
  string card_array;

  for ( int i=0; i<cards_needed; i++ )
    {
      Card* ncard = deck->draw();
      card_array += ncard->bitcode;
      cout << ncard->bitcode +31 << endl;
      //Add card to playing deck
      playing_deck->add_card( ncard );
    }

  return card_array;
}

vector<Set*>find_sets ( vector<Card*>cards )
{
  vector<Card*>test_set;
  vector<Set*>_sets;
  
  for ( int i = 0; i<cards.size(); i++ )
    {
      for ( int j = i+1; j<cards.size(); j++)
	{
	  for ( int k = j+1; k<cards.size(); k++ )
	    {
	      test_set.push_back( cards[i] );
	      test_set.push_back( cards[j] );
	      test_set.push_back( cards[k] );
	      
	      if ( check_set ( test_set ) )
		{
		  Set* nset = new Set;
		  nset->x = i;
		  nset->y = j;
		  nset->z = k;
		  _sets.push_back(nset);
		}
	      test_set.clear();
	    }
	}
    }
  
  return _sets;
}
