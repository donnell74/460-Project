#include "cardlib.h"
#include <vector>
#include <iostream>
#include <string.h> 
#include <random>

using namespace std;

random_device rd;
default_random_engine default_engine( rd() );



void display_card( Card* ncard )
{
    cout << " Number:" << ncard->number << " Symbol:" << ncard->symbol 
         << " Shade:" << ncard->shade << " Color:" << ncard->color 
         << " Bitcode:" << ncard->bitcode << endl;
}


//Random number generator wrapper
int rand( int limit )
{
    uniform_int_distribution<int> uniform_dist( 0, limit );
    int random_number = uniform_dist( default_engine );

    return random_number;
}


Deck::Deck()
{
    //Initiliazing deck object
    for ( int i = 0; i < 3; i++ )
    {
        for ( int j = 0; j < 3; j++ )
        {
            for ( int k = 0; k < 3; k++ )
            {
                for ( int l = 0; l < 3; l++ ) 
                {
                    Card* ncard = new Card;
                    ncard->number = static_cast<Number>( i );
                    ncard->symbol = static_cast<Symbol>( j );
                    ncard->shade = static_cast<Shade>( k );
                    ncard->color = static_cast<Color>( l );
		    
		    //Fixes issue with null char as bitcode
		    if ( i == 0 && j == 0 && k == 0 && l == 0 )
		      {
			ncard->bitcode = (char)200;
		      }
 
		    else
		      {
			ncard->bitcode = ( i << 6 ) | ( j << 4 ) | 
                                     ( k << 2 ) | l;
		      }
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
    for ( int  i = 0; i < 12; i++ )
    {
        cards.push_back(nullptr);
    }
 
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
      {
	  int size = 0;
	  for ( int i = 0; i < 12; i++ )
	  {
	      if ( cards[i] != nullptr )
	      {
		  size +=1;
	      }
	  }
	  return size;
      }

    default:
      break;
    } 

    return cards.size(); 
}


void Deck::reset_top()
{
    top -= 12;
    _count += 12;
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
        for ( int i = initial_count; i < limit; i++ )
        {
            if ( cards[i] != nullptr )
            {
	        cout << "Deck[" << j << "] Number:" << cards[i]->number 
                     << " Symbol:" << cards[i]->symbol << " Shade:" 
                     << cards[i]->shade << " Color:" << cards[i]->color 
                     << "Bitcode:" << cards[i]->bitcode << endl;
	        j++;
            }
        }

        cout << "Card Count:" << count( code ) << endl;
        cout << "x----------------------------------------------x" << endl;
  
        if ( code == 0 )
        {
            cout << "top:" << top << endl;
            cout << "virtual count:"<< _count << endl;
        }
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
        //Pointer to Card struct for temporary storage
        Card* temp;
        for ( int i = _count - 1; i > top; i-- )
	{
	    int random_number = rand(i);

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


void Deck::mem_display()
{
    //Deck Memory Address
    for ( int i = top; i < _count; i++ )
    {
      cout << "Deck[" << i + 1 << "]"<< cards[i] << endl;
    }
}


Card* Deck::draw()
{
    Card* ncard = cards[top];
    top += 1;
    _count -= 1;
    return ncard;
}


void Deck::remove_card( int index )
{
    //Should only be called on playing deck
    cards[index] = nullptr;
    cout << "Removed card at index " << index << endl;
}


void Deck::replace_card( int index, Card* ncard )
{
    cards[index] = ncard;
}


void Deck::clear_cards()
{
    for ( int i = 0; i < 12; i++ )
    {
        cards[i] = nullptr;
    }
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
        if ( top == 81 )
	{
	    return true;
	}
        break;
    
    case 1:
    {
	bool empty = true;
	
	for ( auto card_t : cards )
	{
	    if ( card_t != nullptr )
	    {
		empty = false;
	    }
        }

        return empty;
	break;
    }
      
 
    default:
      break;
    }

    return false;
}


//Boolean comparison card set functions
//Difference comparisons
bool color_differs( vector<Card*>set )
{
    //Returns false if all colors are the same
    return set[0]->color != set[1]->color && set[0]->color != 
           set[2]->color && set[1]->color != set[2]->color; 
}


bool symbol_differs( vector<Card*>set )
{
    //Returns false if all symbols are the same
    return set[0]->symbol != set[1]->symbol && set[0]->symbol != 
           set[2]->symbol && set[1]->symbol != set[2]->symbol; 
}


bool shade_differs( vector<Card*>set )
{
    //Returns false if all shades are the same
    return set[0]->shade != set[1]->shade && set[0]->shade != 
           set[2]->shade && set[1]->shade != set[2]->shade;
}


bool number_differs( vector<Card*>set )
{
    //Returns false if at least two numbers are the same
    return set[0]->number != set[1]->number && set[0]->number != 
           set[2]->number && set[1]->number != set[2]->number;
}


//Same comparisons
bool color_same( vector<Card*>set )
{
    //Returns false if all colors are not the same
    return set[0]->color == set[1]->color && set[0]->color == 
           set[2]->color && set[1]->color == set[2]->color;
}


bool symbol_same( vector<Card*>set )
{
    //Returns false if all symbols aren't the same
    return set[0]->symbol == set[1]->symbol && set[0]->symbol == 
           set[2]->symbol && set[1]->symbol == set[2]->symbol;
}


bool shade_same( vector<Card*>set )
{
    //Returns false if all shades arent the same
    return set[0]->shade == set[1]->shade && set[0]->shade == 
           set[2]->shade && set[1]->shade == set[2]->shade;
}


bool number_same( vector<Card*>set )
{
    //Returns false if all numbers arent the same
    return set[0]->number == set[1]->number && set[0]->number == 
           set[2]->number && set[1]->number == set[2]->number;
}


//Checks if array of sent cards is set
bool check_set ( vector<Card*>set )
{
    /*Logic - set must meet all conditions to be a valid set
    They all have the same number, or they have three different numbers.
    They all have the same symbol, or they have three different symbols.
    They all have the same shading, or they have three different shadings.
    They all have the same color, or they have three different colors.*/
  
    if( ( number_same( set ) || number_differs( set ) ) && 
        ( symbol_same( set ) || symbol_differs( set ) ) && 
        ( shade_same( set )  || shade_differs( set ) )  && 
        ( color_same( set )  || color_differs( set ) ) )
    {
         return true;
    }
    
    else
    {
        return false;
    }
  
}


void set_auxillary()
{
    string attr[] = { "color", "symbol", "shade", "number" };
    string prompts[] = { "Color:  red - 0\n\tpurple - 1\n\tgreen - 2",
                         "symbol: cross - 0\n\tsquare - 1\n\tslash - 2",
                         "shade:  circle - 0\n\tat - 1\n\tspace - 2",
                         "Number: one - 0\n\ttwo - 1\n\tthree - 2" };
 
    vector<Card*>set;
    int mcards = 3;
  
    for( int j = 0; j < mcards; j++ )
    {
        int ins[4];
        Card* ncard = new Card;

        for ( int i=0; i<4; i++ )
	{
	    cout << prompts[i] << endl;
	    cout << "Card "<< j + 1 << " " << attr[i] << ":" << endl;
	    cin >> ins[i];
	}

        ncard->color = static_cast<Color>( ins[0] );
        ncard->symbol = static_cast<Symbol>( ins[1] );
        ncard->shade = static_cast<Shade>( ins[2] );
        ncard->number = static_cast<Number>( ins[3] );
        set.push_back( ncard );
        cout << "card_color:"  << set[j]->color  << endl;
        cout << "card_symbol:" << set[j]->symbol << endl;
        cout << "card_shade:"  << set[j]->shade  << endl;
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


string create_playing_cards( vector<int>indexes, 
                             Deck* deck, Deck* playing_deck )
{  
    string card_array;
  
    for ( unsigned int i = 0; i < indexes.size(); i++ )
    {
        Card* ncard = deck->draw();
        card_array += ncard->bitcode;
	card_array += ":" + to_string(indexes[i]) + ";";
        cout << ncard->bitcode + 31 << endl;
        //Add card to playing deck
        playing_deck->replace_card( indexes[i], ncard );
    }
 
    return card_array;
}


vector<Set*> find_sets ( vector<Card*>cards )
{
    vector<Card*>test_set;
    vector<Set*>_sets;
  
    for ( unsigned int i = 0; i < cards.size(); i++ )
    {
        for ( unsigned int j = i + 1; j < cards.size(); j++)
	{
	    for ( unsigned int k = j + 1; k < cards.size(); k++ )
	    {
	        if ( cards[i] != nullptr && cards[j] != nullptr && 
                     cards[k] != nullptr )
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
		        _sets.push_back( nset );
		    }
		    test_set.clear();
		}
	    }
	}
    }
  
    return _sets;
}


int num_sets ( vector<Card*>cards )
{
    vector<Set*>_sets = find_sets( cards );
    return _sets.size();
}


void display_sets ( vector<Card*>cards )
{
    vector<Set*>_sets = find_sets( cards );
  
    if ( _sets.size() == 0 )
    {
        cout << "No valid sets in cards" << endl;
        return;
    }

    cout << _sets.size() << " possible sets" << endl;
    for ( unsigned int i = 0; i < _sets.size(); i++ )
    {
        cout << "Deck[" << _sets[i]->x << "] Deck[" << _sets[i]->y 
             << "] Deck[" << _sets[i]->z << "]" << endl;
    }
}


int map_card ( char key )
{
    switch ( key )
    {

      case '1':
          return 0;

      case '2':
          return 1;

      case '3':
          return 2;

      case '4':
          return 3;

      case 'Q':
          return 4;

      case 'W':
          return 5;

      case 'E':
          return 6;

      case 'R':
          return 7;

      case 'A':
          return 8;

      case 'S':
          return 9;

      case 'D':
          return 10;

      case 'F':
          return 11;

      /*Required if client list is extended to 16
      case 'Z':
          return 12;

      case 'X':  
          return 13;

      case 'C':
          return 14;

      case 'V':
          return 15;
*/
    default:
      return 0;
    }
}
