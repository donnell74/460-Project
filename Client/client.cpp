#define _POSIX_SOURCE
/* cpp includes */
#include <iostream>
#include <vector>
#include <algorithm>
#include <string.h>

/* local includes */
#include "networking.h"
#include "cardlib.h"
using namespace std;

/* Constants */
const char EXIT = 'x';
const char MESSAGE = 'm';
const char CARDS = 'c';

// last line of ACCEPTED_CHARS is for command chars
//Rev. 'O' and 'N' used for 'nos' command in a no set valid 
//response
vector<char> ACCEPTED_CHARS = {'1', '2', '3', '4', 
                               'Q', 'W', 'E', 'R',
                               'A', 'S', 'D', 'F', 
                               'Z', 'X', 'C', 'V',
                               'O', 'N', 'U', 'I',
			       'T'}; 
Client *my_client;


void sig_wrap_cleanup( int sig )
{
  // wrapper for sigaction to pass int to sig which is never used
  my_client->cleanup();
}


void handle_input()
{
  string inp = "";
  cin >> inp;
  cin.get(); // remove newline

  if ( inp.size() < (size_t) 3 )
  {
    cout << "Not using all three inputs will never be a set." << endl;
    return;
  }

  for ( auto it = inp.begin(); it != inp.end(); ++it )
  {
    *it = toupper(*it);
    if ( find(ACCEPTED_CHARS.begin(), ACCEPTED_CHARS.end(), *it) == 
         ACCEPTED_CHARS.end() )
    {
      cout << "Key " << *it << " is not allowed." << endl;
      return;
    }
  } 

  // substr will only get first 3, stop long string entries

  // check if any repeats
  if ( inp[0] == inp[1] ||
       inp[1] == inp[2] ||
       inp[0] == inp[2] )
  {
    cout << "Repeats are not allowed." << endl;
  }
  
  else
    {
      my_client->send_message( inp.substr(0,3) );
    }
}


void handle_server_msg()
{
  string msg = my_client->get_next_msg();

  switch( msg.front() )
  {
    case EXIT:
      cout << msg.substr( 1 ) << endl;
      my_client->cleanup();
      break;

    case MESSAGE:
      cout << "Message Recieved: " << endl;
      cout << msg.substr( 1 ) << endl; 
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
  }

}
