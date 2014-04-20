#define _POSIX_SOURCE
/* Cpp inlcudes */
#include <iostream>
#include <vector>
/* C & linux incldues */
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <poll.h>
#include <pthread.h>
/* Dependencies */
#include "server.h"
#include "cardlib.h"
using namespace std;

/* Globals */
Server *my_server;
pthread_mutex_t mutex;

/* Never used, here because of other bad code */
void handle_server_msg ()
{
  return;
}


void display_options ( )
{
  cout << "Menu" << endl;
  cout << "B. Start Game" << endl;
  cout << "Q. Quit" << endl;
  cout << "D. Display deck" << endl;
  cout << "S. Shuffle deck" << endl;
  cout << "A. Memory Addresses" << endl;
  cout << "W. Display possible sets" << endl;
  cout << "P. Display Playing Deck" << endl;
  //cout << "K. Set Options" << endl;
 }

/*void set_options ( )
{
  cout << "Set Options" << endl;
  cout << "1. Test Set" << endl;
  cout << "2. Show all sets" << endl;
}
*/

void handle_input()
{
  char type = 0;
  string inp = {};

  cin >> inp;
  type = inp[0];
  cout << type << endl;
  switch( toupper( type ) )
  {
  
    case 'B':
      my_server->send_playing_cards( std_indexes );
      display_sets( my_server->playing_deck->get_cards() );
      break;

    case 'M':
      display_options();
      break;

    case 'Q':
      {
        my_server->~Server();
      }
      break;

    case 'D':
      {
        my_server->deck->display( 0 );
      }
      break;
      
    case 'S':
      {
        cout<<"shuffling..."<<endl;
        my_server->deck->shuffle();
      }
      break;

    case 'A':
      {
        my_server->deck->mem_display();
      }
      break;
    
    case 'W':
      {
	display_sets( my_server->playing_deck->get_cards() );
      }
      break;
      
  case 'P':
    {
      my_server->playing_deck->display( 1 );
    }
    break;

  case 'U':
    {
      my_server->update_scores();
      cout << "Updating" << endl;
    }
  
  default:
    break;
  }
}

void *wait_for_client_wrap ( void *arg )
{
  my_server->wait_for_client();

  return arg;
}


void sig_wrap_cleanup ( int sig )
{
  my_server->~Server();
}


int main(int argc, char* argv[])
{
  char LOCALHOST[] = "127.0.0.1";
  int DELAY = 15;

  if ( argc == 1 || argc > 3 )
  {
	cerr << "Usage: ./server <port>" << endl;
        exit(EXIT_SUCCESS); 
  }

  else
  {
    if ( argc == 2 ) // No Addr or Dealy given
    {
      my_server = new Server( atoi( argv[1] ), LOCALHOST, DELAY );
    }

    if ( argc == 3 ) // Addr and Port Given, but no Dealy
    {
      my_server = new Server( atoi( argv[1] ), LOCALHOST, atoi(argv[2]) );
    }
    
  }

  //Initialize deck and playing deck
  my_server->deck = new Deck();
  my_server->playing_deck = new Deck( 1 );

  //Shuffle deck
  my_server->deck->shuffle();
  my_server->deck->shuffle();

  // bind TERM to cleanup
  struct sigaction action = {};

  action.sa_handler = sig_wrap_cleanup;
  sigaction( SIGINT, &action, nullptr );
  
  pthread_t thread;
  if ( pthread_create( &thread, NULL, wait_for_client_wrap, nullptr ) != 0 )
  {
    my_server->die( "Couldn't start wait for client" );
  }

  pthread_detach( thread );

  my_server->wait_for_input();
  
  my_server->cleanup();
}

