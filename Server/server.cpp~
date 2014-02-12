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
#include "networking.h"
#include "cardlib.h"
using namespace std;

/* Globals */
Deck *deck;
Deck *playing_deck = new Deck(1);
Server *my_server;
pthread_mutex_t mutex;

//Creates the array of cards from playing_deck
//to send to client
string create_array ( int cards_needed )
{
  string card_array;

  for ( int i=0; i<cards_needed; i++ )
    {
      Card *ncard = deck->draw( );
      card_array += ncard->bitcode + 31;
      cout << ncard->bitcode + 31 << endl;

      //Add card to playing deck
      playing_deck->add_card( ncard );
    }
  
  return card_array;
}


// Sends desired amount of cards to all clients.
void send_playing_cards (  ) 
{ 
  int cards_needed = 12;
  string cards_to_send = create_array ( cards_needed );
  for ( auto client_it : my_server->get_client_list() )
  {
    my_server->sendMessage( client_it.sock_fd, 'c', cards_to_send ); 
  }
}



void display_options ( )
{
  cout << "Q. Quit" << endl;
  cout << "D. Display deck" << endl;
  cout << "S. Shuffle deck" << endl;
  cout << "A. Memory Addresses" << endl;
  cout << "W. Draw card"<< endl;
  cout << "P. Print Playing Cards" << endl;
}


void handle_input()
{
  char type = 0;
  string inp = {};

  cin >> inp;
  type = inp[0];
  cout << type << endl;
  switch( toupper( type ) )
  {
    case 'Q':
      {
        my_server->cleanup();
      }
      break;

    case 'D':
      {
        deck->display( 0 );
      }
      break;
      
    case 'S':
      {
        cout<<"shuffling..."<<endl;
        deck->shuffle();
      }
      break;

    case 'A':
      {
        deck->mem_display();
      }
      break;
    
    case 'W':
      {
        Card *ncard = deck->draw();
        playing_deck->add_card( ncard );
        
        cout << "Drew card:" << endl;
        cout << "Symbol:" << ncard->symbol << " Shade:" << ncard->shade << " Color:" << ncard->color << " Number:" << ncard->number << endl;
      }
      break;

    case 'P':
    {
      string card_array;
      int cards_needed;

      cout << "cards needed?" <<endl;
      cin >> cards_needed;
      
      card_array = create_array( cards_needed );
      cout << card_array <<endl;

      playing_deck->display( 1 );
    }
    break;

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
  my_server->cleanup();
}


int main(int argc, char* argv[])
{
  deck = new Deck();
  char LOCALHOST[] = "127.0.0.1";

  if ( argc ==1 || argc > 3 )
  {
    my_server->die("Usage: ./server <port>");
  }
  else
  {
    if ( argc == 2) // No port number given
    {
      my_server = new Server( atoi( argv[1] ), LOCALHOST );
    }

    if ( argc == 3) // Addr and Port Given
    {
      my_server = new Server( atoi( argv[1] ), argv[2] );
    }
  }

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

