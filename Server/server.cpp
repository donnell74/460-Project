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
/* Dependencies */
#include "cardlib.h"
using namespace std;

/* structs - move to header file later */
struct Client
{
  int sock_fd;  
  sockaddr_in addr;
};

/* Constants */
int DEBUG = 1;
int DEFAULT_PORT = 51717;
char LOCALHOST[] = "127.0.0.1";
int MAXCLIENTS = 25;

/* Globals */
int server_sock_fd = 0;
vector<Client> client_list;
vector<pollfd> poll_fds {{STDIN_FILENO, POLLIN, 0}};


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
  int bytes_sent = write( sock_fd, msg.c_str(), msg.size()); 
  if ( bytes_sent < 0 )
  {
    die( "Unable to send message." );
  }
}


void cleanup ()
{
  cout << "Server going down (as well as all clients)." << endl;
  for ( auto client_it : client_list )
  {
    sendMessage( client_it.sock_fd, 'x', "You have been disconnected"); 
  }
  if ( close( server_sock_fd ) == -1  && DEBUG )
  {
    cerr << strerror( errno ) << endl;
  }
  exit(EXIT_SUCCESS);
}


void init_server ( int port, char *addr )
{
  cout << "Entering init_server." << endl;
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
}


void wait_for_client ()
{
  // will need to be edited for mulithreading
  // currently just connects one client
  // insert at begining because it doesn't matter position of client in 
  // list but placing in front is faster
  auto client_it = client_list.emplace( client_list.begin(), Client{});
  // emplace might be replacing, check when implementing multiple client
  socklen_t client_len = sizeof (client_it->addr);
  client_it->sock_fd = accept(server_sock_fd,
                                  (struct sockaddr *) &client_it->addr,
                                  &client_len );

  if ( client_it->sock_fd < 0 )
  {
    die("Error on accept. ");
  }
  else
  {
    cout << "Connected new client." << endl;

    // add poll fd and send message
    struct pollfd client_sock_fd = {client_it->sock_fd, POLLIN, 0};
    poll_fds.push_back(client_sock_fd);
    sendMessage( client_it->sock_fd, 'm', "You have been connected.");
  }
}


void handle_input()
{
  char type = 0;
  string inp = {};

  cin >> inp;
  type = inp[0];
  switch( toupper( type ) )
  {
    case 'Q':
      cleanup();
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
  close( client_sock_fd );
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


void wait_for_input ()
{
  for ( ;; )
  {
    // &poll_fds[0] can cause problems on reallocation
    if ( poll( &poll_fds[0], poll_fds.size(), -1 ) == -1 )
    {
      die("Problem with input.");
    }    
    else
    {

      if ( ( poll_fds[0].revents & POLLIN ) != 0 )
      {
        handle_input();
      }

      if ( ( poll_fds[1].revents & POLLIN ) != 0 )
      {
        // probally handle new connection?
      }

      if ( poll_fds.size() > 2 )
      {
        for ( auto poll_fd_it : poll_fds )
        {
          if ( poll_fd_it.fd < 4 )
          {
            continue;
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

//Initializer for Deck struct. Returns a deck struct pointer
Deck* deck_init()
{
  Deck* ndeck = new Deck;
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

  for(int i=1; i<4; i++)
    {
      for(int j=0; j<3; j++)
	{
	  for(int k=0; k<3; k++)
	    {
	      for(int l=0; l<3; l++)
		{
		  Card* ncard = new Card;
		  ncard->number = i;
		  ncard->symbol = symbols[j];
		  ncard->shade = shade[k];
		  ncard->color = colors[l];
		  ncard->bitcode = 0;
		  ndeck->cards.push_back(ncard);
		}
	    }
	}
    }
  
  ndeck->card_count = ndeck->cards.size();

  return ndeck;
}

//Shuffles the deck using Fisher-Yates Shuffle
void shuffleDeck(Deck* deck)
{
  random_device rd;
  default_random_engine e1(rd());
  
  //Pointer to Card struct for temporary storage
  Card* temp;
  for(int i=deck->card_count-1; i>0; i--)
    {
      uniform_int_distribution<int> uniform_dist(0,i);
      int random_number = uniform_dist(e1);

      temp = CARD_REF;
      CARD_REF = deck->cards[random_number];
      deck->cards[random_number] = temp;
    }

}

//Displays the current contents of a deck struct
void displayDeck(Deck* deck)
{
  if(deck == nullptr)
    {
      cout<<"Invalid deck pointer"<<endl;
      exit(EXIT_FAILURE);
    }

  for(int i=0; i<deck->card_count; i++)
    {
      cout<<"Deck["<<i+1<<"]:"<<CARD_REF->number<<" "<<CARD_REF->symbol<<" "<<CARD_REF->shade<<" "<<CARD_REF->color<<endl;
    }

  cout<<"x----------------------------------------------x"<<endl;
}


void memoryAddresses(Deck* deck)
{
  //Deck Memory Address
  cout<<"Deck Memory Address:"<<&deck<<endl;
  for(int i=0; i<deck->card_count; i++)
    {
      cout<<"Deck["<<i+1<<"]"<<deck->cards[i]<<endl;
    }
}

int main(int argc, char* argv[])
{
  //Initialize new deck struct
  Deck* deck = deck_init();
  int choice;
  bool cont = true;
  
  //Auxillary testing functions
  while(cont)
    {
      cout<<"1. Display deck"<<endl;
      cout<<"2. Shuffle deck"<<endl;
      cout<<"3. Memory Addresses"<<endl;
      cin>>choice;

      switch(choice)
	{
	case 1:
	  displayDeck(deck);
	  break;
	case 2:
	  cout<<"shuffling..."<<endl;
	  shuffleDeck(deck);
	  break;
	case 3:
	  memoryAddresses(deck);
	  break;
	default:
	  cont = false;
	 }
    }

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

  wait_for_client();
  wait_for_input();
  cleanup();
}

