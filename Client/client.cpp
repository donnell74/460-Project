#define _POSIX_SOURCE
/* cpp includes */
#include <iostream>
#include <vector>
#include <algorithm>
#include <curses.h>
#include <cstdlib>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
/* local includes */
#include "networking.h"
#include "cardlib.h"
using namespace std;

/* Constants */
const char EXIT = 'x';
const char MESSAGE = 'm';
const char CARDS = 'c';

// last line of ACCEPTED_CHARS is for command chars
<<<<<<< HEAD
//vector<char> ACCEPTED_CHARS = {'1', '2', '3', '4', 
//                               'Q', 'W', 'E', 'R',
//                               'A', 'S', 'D', 'F', 
//                               'Z', 'X', 'C', 'V',
//                               'U', 'I', 'T'};

vector<char> ACCEPTED_CHARS = {'A', 'B', 'C', 'D',
                               'E', 'F', 'G', 'H',
                               'I', 'J', 'K', 'L'}; 
=======
//Rev. 'O' and 'N' used for 'nos' command in a no set valid 
//response
vector<char> ACCEPTED_CHARS = {'1', '2', '3', '4', 
                               'Q', 'W', 'E', 'R',
                               'A', 'S', 'D', 'F', 
                               'Z', 'X', 'C', 'V',
                               'O', 'N', 'U', 'I',
			       'T'}; 
>>>>>>> 0499b1edba32f4069a3e434ac4fb52542f4ed35f
Client *my_client;


void sig_wrap_cleanup( int sig )
{
  // wrapper for sigaction to pass int to sig which is never used
  my_client->cleanup();
}


//|<handle_input>
void handle_input()
{
  cbreak();
  noecho();
   
  vector<char> select = {};
  
  for ( int i = 0; ; ++i )
  { 

    int c = getch();

    if ( c == ERR || ( c == ' ' && select.size() == 3 ) ) 
    {
      break;
    }
    
    c = toupper( c );

    //If already selected, deselect it and go to top.
    if ( find( select.begin(), select.end(), c ) != select.end() )
    {
       //add code to deselect the card here
       select.erase( remove( select.begin(), select.end(), c ), select.end() );
       refresh();
       continue;
    }

    //If acceptable char and not a selected card, select it.
    if ( find(ACCEPTED_CHARS.begin(), ACCEPTED_CHARS.end(), c ) != 
         ACCEPTED_CHARS.end() && 
         find (select.begin(), select.end(), c ) == select.end() )  
    {
      //add code to select card here
      select.push_back( c );
      refresh();
    }
  }
<<<<<<< HEAD

  string inp( select.begin(), select.end() );
  printw( "%s", inp.data() );
  printw( "%s", "\n" );
  echo();
  refresh();

 
  //Send substing in case of left overs.
  //my_client->send_message( inp.substr( 0,3 ) );
  my_client->send_message( inp );
=======
  
  else
    {
      my_client->send_message( inp.substr(0,3) );
    }
>>>>>>> 0499b1edba32f4069a3e434ac4fb52542f4ed35f
}


void handle_server_msg()
{
  string msg = my_client->get_next_msg();

  switch( msg.front() )
  {
    case EXIT:
      printw( "%s", msg.substr( 1 ).data() );
      printw( "%s", "\n" );
      refresh();
      my_client->cleanup();
      break;

    case MESSAGE:
      printw( "%s", "Message Received: \n" );
      printw( "%s", msg.substr( 1 ).data() );
      printw( "%s", "\n" );
      refresh(); 
      break;

    case CARDS:
      if ( DEBUG )
      {
	cout << "CARDS NOM NOM NOM" << endl;
	cout << "Size: " << msg.size() << endl;
	for ( int i = 1; i < (int) msg.size(); ++i )
	{
          cout << "Recieved card with bitcode: " << int( msg[i] ) << endl;
	}
      }
      break;

    default:
      break;
  }

  if ( ! my_client->get_past_data_read().empty() )
  {
    handle_server_msg ();
  }
}


int main( int argc, char *argv[] )
{
  char LOCALHOST[] = "127.0.0.1";

  initscr();

  if ( argc == 1 || argc > 3 )
  {
    my_client->die("Usage: ./client <port>");
  }
  else
  {
    if ( argc == 2 )
    {
      my_client = new Client( atoi( argv[1] ), LOCALHOST );
    }
    else
    {
      my_client = new Client( atoi( argv[1] ), argv[2] );
    }  
    
    // bind TERM to cleanup
    struct sigaction action = {};

    action.sa_handler = sig_wrap_cleanup;
    sigaction( SIGINT, &action, nullptr );

    my_client->wait_for_input();
    my_client->cleanup();

    //getch();
    endwin();
  }

}
