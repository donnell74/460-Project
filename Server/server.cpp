#define _POSIX_SOURCE
/* Cpp inlcudes */
#include <iostream>
#include <vector>
/* C & linux incldues */
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <poll.h>
#include <signal.h>
#include <pthread.h>
/* Dependencies */
#include "networking.h"
#include "cardlib.h"
using namespace std;

/* Globals */
int server_sock_fd = 0;
Deck *deck;
Deck *playing_deck = new Deck(1);
vector<Client> client_list;
vector<pollfd> poll_fds {{STDIN_FILENO, POLLIN, 0}};
pthread_mutex_t mutex;

void die ( string error_msg )
{
  // standardize error_msg so no extra new lines
  error_msg.erase(error_msg.find_last_not_of(" \n\r\t")+1); 
  cerr << error_msg << endl;

  if ( DEBUG )
  {
    cerr << strerror( errno ) << endl;
  }
  
  // close socket to server
  if ( close( server_sock_fd ) == -1 && DEBUG )
  {
    cerr << strerror( errno ) << endl;
  }
  exit(EXIT_FAILURE);
}


void sendMessage( int sock_fd, char type, string msg )
{
  msg.insert(msg.begin(), type); // makes calls to sendMessage more readable
  msg.append( TERM_STR ); 
  int bytes_sent = write( sock_fd, msg.c_str(), msg.size()); 
  if ( bytes_sent < 0 )
  {
    die( "Unable to send message." );
  }
}


void cleanup ()
{
  cout << "Server going down (as well as all clients)." << endl;
  // client list is empty or we quit while creating first client connection
  if ( ! client_list.empty() && ( client_list.front().sock_fd != 0 ) )
  {
    for ( auto client_it : client_list )
    {
      cout << "Closing client " << client_it.sock_fd << endl;
      sendMessage( client_it.sock_fd, 'x', "You have been disconnected"); 
    }
  }
  
  cout << "All clients should be closed." << endl;
  cout << "Now closing server." << endl;

  if ( close( server_sock_fd ) == -1  && DEBUG )
  {
    cerr << strerror( errno ) << endl;
  }

  cout << "Server should now be closed." << endl;
  exit(EXIT_SUCCESS);
}


void sig_wrap_cleanup( int sig )
{
  // wrapper for sigaction to pass int sig which is never used
  cleanup();
}


void init_server ( int port, char *addr )
{
  // create socket for server
  server_sock_fd = socket( AF_INET, SOCK_STREAM, 0 );
  if ( server_sock_fd < 0 )
  {
    die("Error opening socket. ");
  }

  // add server socket to poll_fds
  struct pollfd server_pollfd = {server_sock_fd, POLLIN, 0};
  poll_fds.push_back(server_pollfd);

  // create sock addr we use for the rest of program
  struct sockaddr_in server_addr = {};
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons( port );

  // convert ip text string to binary
  if ( inet_pton( AF_INET, addr, &server_addr.sin_addr ) != 1 )
  {
    die("Error converting addr during server initialization. ");
  }  

  int yes=1;
  
  // lose the pesky "Address already in use" error message
  if (setsockopt(server_sock_fd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
      die("setsockopt");
  } 
  
  // bind socket to the server ip
  if ( bind( server_sock_fd, 
             (struct sockaddr *) &server_addr,
             sizeof server_addr ) < 0)
  {
    die("Can't bind to server socket. ");
  }

  // start listening for a connection
  // change SOMAXCON to a int if requirements
  // require a limit on clients
  if ( listen( server_sock_fd, MAXCLIENTS ) < 0 )
  {
    die( "Unable to listen." );
  }

  // output that it is listening
  cout << "Listening on port " << addr << endl;

  // bind TERM to cleanup
  struct sigaction action = {};

  action.sa_handler = sig_wrap_cleanup;
  sigaction( SIGINT, &action, nullptr );
}


//Creates the array of cards from playing_deck
//to send to client
string create_array ( int cards_needed )
{
  string card_array;

  for ( int i=0; i<cards_needed; i++ )
    {
      Card *ncard = draw( deck );
      card_array += ncard->bitcode;

      //Add card to playing deck
      playing_deck->cards.push_back( ncard );
    }
  
  return card_array;
}


// Sends desired amount of cards to all clients.
void send_playing_cards (  ) 
{ 
  int cards_needed = 12;
  string playing_cards = create_array ( cards_needed );
  cout << "Server sending playing cards now..." << endl;
  cout << "This is client list lenght: " << client_list.size() << endl;
  for ( auto client_it : client_list )
  {
    sendMessage( client_it.sock_fd, 'c', playing_cards ); 
  }
}


void *wait_for_client ( void *arg )
{
  for (;;)
  {
    // will need to be edited for mulithreading
    // currently just connects one client
    // auto client_it = client_list.push_back( client_list.begin(), Client{});
    // emplace might be replacing, check when implementing multiple client
    Client this_client = {};
    socklen_t client_len = sizeof (this_client.addr);
    this_client.sock_fd = accept(server_sock_fd,
                                    (struct sockaddr *) &this_client.addr,
                                    &client_len );

    if ( this_client.sock_fd < 0 )
    {
      die("Error on accept. ");
    }
    else
    {
      cout << "Connected new client." << endl;

      // add poll fd and send message
      struct pollfd client_sock_fd = {this_client.sock_fd, POLLIN, 0};

      // CRITICAL SECTION
      pthread_mutex_lock(&mutex);
      poll_fds.push_back(client_sock_fd);
      client_list.push_back( this_client );
      pthread_mutex_unlock(&mutex);

      sendMessage( this_client.sock_fd, 'm', "You have been connected.");
      send_playing_cards( );
      //sendMessage( this_client.sock_fd, 'c', "" + deck->cards[0]->bitcode );
    }
  }

  return arg;
}



//Creates the array of cards from playing_deck
//to send to client
string create_array ( int cards_needed )
{
  string card_array;

  for ( int i=0; i<cards_needed; i++ )
    {
      Card *ncard = deck->draw();
      card_array += ncard->bitcode;

      //Add card to playing deck
      playing_deck->add_card( ncard );
    }
  
  return card_array;
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
        cleanup();
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


void disconnect_client( int client_sock_fd )
{
  for ( auto client_it = client_list.begin(); client_it != client_list.end(); ++client_it )
  {
    if ( client_it->sock_fd == client_sock_fd )
    {
      client_list.erase( client_it );
      break;
    }
  }

  for ( auto poll_fd_it = poll_fds.begin(); poll_fd_it != poll_fds.end(); ++poll_fd_it )
  {
    if ( poll_fd_it->fd == client_sock_fd )
    {
      poll_fds.erase( poll_fd_it );
      break;
    }
  }

  // even if close fails, server acts if client has died
  // as of this point
  sendMessage( client_sock_fd, 'x', "You have been disconnected"); 
  cout << "Client with fd of " << client_sock_fd << " has disconnected." << endl;
}


void recieve_input( int client_sock_fd )
{
  int bytes_read = 0;
  char buffer[4] = {0}; // will only care about first 3 if guess

  bytes_read = read( client_sock_fd, &buffer, 3);
  if ( bytes_read != -1 )
  {
    if ( strcmp( buffer, "QUI" ) == 0 )
    {
      disconnect_client( client_sock_fd );
    }
    else
    {
      cout << "Guess from " << client_sock_fd << " of "
           << buffer << endl;
    }
  }
  else
  {
    if ( DEBUG )
    {
      cout << "Problem with read from client." << endl;
    }
  }
}


void reload_poll_fds ( )
{
  // this function does nothing
}


void wait_for_input ()
{
  for ( ;; )
  {
    // &poll_fds[0] can cause problems on reallocation
    if ( poll( &poll_fds[0], poll_fds.size(), 1 ) == -1 )
    {
      die("Problem with input.");
    }    
    else
    {

        if ( ( poll_fds[0].revents & POLLIN ) != 0 )
      {
        handle_input();
      }

      // POLLRDHUP
      if ( poll_fds.size() > 2 )
      {
        for ( auto poll_fd_it : poll_fds )
        {
          if ( poll_fd_it.fd < 3 || poll_fd_it.fd == server_sock_fd )
          {
            continue;
          }

          if ( ( poll_fd_it.revents & POLLRDHUP ) != 0 )
          {
            // client hung up
            disconnect_client( poll_fd_it.fd );
          }

          if ( ( poll_fd_it.revents & POLLIN ) != 0 )
          {
            recieve_input( poll_fd_it.fd );
          }
        }
      }
    }
  }
}


int main(int argc, char* argv[])
{
  deck = new Deck();

  if ( argc ==1 || argc > 3 )
  {
    die("Usage: ./server <port>");
  }
  else
  {
    if ( argc == 2) // No port number given
    {
      init_server( atoi( argv[1] ), LOCALHOST );
    }

    if ( argc == 3) // Addr and Port Given
    {
      init_server( atoi( argv[1] ), argv[2] );
    }
  }

  pthread_t thread;
  if ( pthread_create( &thread, NULL, wait_for_client, nullptr ) != 0 )
  {
    die( "Couldn't start wait for client" );
  }
  pthread_detach( thread );

  wait_for_input();
  cleanup();
}

