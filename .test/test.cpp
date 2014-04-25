#include<vector>
#include<iostream>
#include<random>
#include<unistd.h>
#include<string>
#define TOP 0
#define COUNT 81
using namespace std;
  
random_device rd;
default_random_engine default_engine( rd() );
vector<string>cards;

int rand( int limit )
{
    uniform_int_distribution<int> uniform_dist( 0, limit );
    int random_number = uniform_dist( default_engine );

    return random_number;
}

void initcards( )
{
  for( int i=0; i<81; i++ )
    {
      cards.push_back(to_string(i));
    }
}

int main( int argc, char** argv )
{

  switch( atoi(argv[1]) )
    {
    case 0:
      {
	int limit = 11;
	vector<int>indxs = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
             
	for ( unsigned int i = 0; i < 6; i++ )
	  {
	    indxs.erase( indxs.begin() + rand( limit ) );
	    limit--;
	  }

	for ( unsigned int i = 0; i<indxs.size(); i++ )
	  {
	    cout<<"Index "<<indxs[i]<<endl;
	  }
      }
      break;

    case 1:
      {
	initcards();
        //Pointer to Card struct for temporary storage
        string temp;
        for ( int i = 0; i < COUNT; i++ )
	{
	    int random_number = rand(COUNT - i);
	    temp = cards[i+TOP];
	    cards[i+TOP] = cards[random_number+TOP];
	    cards[random_number+TOP] = temp;
	}

	for ( int i = 0; i < 81; i++ )
	  {
	    cout<<"Card["<<i<<"] "<<cards[i]<<endl;
	  }
      }
      break;
    default:
      break;
    }

  exit(EXIT_SUCCESS);
}
