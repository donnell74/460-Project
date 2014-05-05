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
pthread_t timer_thread;

//end Globals

/* Never used, here because of other bad code */
//|handle_server_msg
void handle_server_msg()
{
    return;
}


//|display_options
void display_options()
{
    cout << "Menu" << endl;
    cout << "B. Start Game" << endl;
    cout << "O. End Game" << endl;
    cout << "Q. Quit" << endl;
    cout << "D. Display deck" << endl;
    cout << "S. Shuffle deck" << endl;
    cout << "A. Memory Addresses" << endl;
    cout << "W. Display possible sets" << endl;
    cout << "P. Display Playing Deck" << endl;
}


//|handle_input
void handle_input()
{
    char type = 0;
    string inp = {};

    cin >> inp;
    type = inp[0];

    switch( toupper( type ) )
    {
  
        case 'B':
	    my_server->begin_game();
            break;

        case 'O':
            my_server->force_game_over();
            break;

        case 'M':
            display_options();
            break;

        case 'Q':
            my_server->~Server();
            break;

        case 'D':
            my_server->deck->display( 0 );
            break;
      
        case 'S':
            cout << "Shuffling..." << endl;
            my_server->deck->shuffle();
            break;

        case 'A':
            my_server->deck->mem_display();
            break;
    
        case 'W':
	    display_sets( my_server->playing_deck->get_cards() );
            break;
      
        case 'P':
            my_server->playing_deck->display( 1 );
            break;

        case 'U':
            my_server->update_scores();
            cout << "Updating..." << endl;
	    break;
       
        default:
            break;
    }
}


//|start_timer_wrapper
void *start_timer_wrap( void *arg )
{
    my_server->start_timer();
    return arg;
}


//|*wait_for_client_wrap
void *wait_for_client_wrap( void *arg )
{
    my_server->wait_for_client();
    return arg;
}


//|sig_alarm_wrap
void sig_alarm_wrapper( int sig )
{
    my_server->check_timer();
    my_server->update_client_timer();
}


//|sig_wrap_cleanup
void sig_wrap_cleanup( int sig )
{
    my_server->~Server();
}


//|main
int main( int argc, char* argv[] )
{
    char LOCALHOST[] = "127.0.0.1";
    int DELAY = 15;

    if ( argc == 1 || argc > 3 )
    {
        cerr << "Usage: ./server <port>" << endl;
        exit( EXIT_SUCCESS ); 
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
  
    //Timer sigaction
    struct sigaction time_action = {};  
    time_action.sa_handler = sig_alarm_wrapper;
    sigaction( SIGALRM, &time_action, nullptr );
  
    //Start timer thread 
    if( pthread_create( &timer_thread, nullptr, start_timer_wrap, 
                        nullptr ) != 0 )
    {
        my_server->die( "Couldn't create timer_thread" );
    }

    pthread_t thread;

    if ( pthread_create( &thread, NULL, wait_for_client_wrap, nullptr ) != 0 )
    {
        my_server->die( "Couldn't start wait for client" );
    }  

    pthread_detach( thread );
    pthread_detach( timer_thread );
    my_server->wait_for_input();
  
    my_server->cleanup();
}

