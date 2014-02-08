#define _POSIX_SOURCE
/* cpp includes */
#include <iostream>
#include <vector>
#include <algorithm>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <poll.h>
#include <netdb.h>
#include <sys/fcntl.h>
#include <signal.h>
using namespace std;

/* Constants */
int DEBUG = 1;
int DEFAULT_PORT = 51717;
char LOCALHOST[] = "127.0.0.1";
string TERM_STR = "XX";
const char EXIT = 'x';
const char MESSAGE = 'm';
const char CARDS = 'c';
// last line of ACCEPTED_CHARS is for command chars
vector<char> ACCEPTED_CHARS = {'1', '2', '3', '4', 
                               'Q', 'W', 'E', 'R',
                               'A', 'S', 'D', 'F', 
                               'Z', 'X', 'C', 'V',
                               'U', 'I', 'T'}; 

/* Globals */
int client_sock_fd = 0;
string past_data_read = "";

void die ( string error_msg )
{
  // standardize error_msg so no extra new lines
  error_msg.erase(error_msg.find_last_not_of(" \n\r\t")+1); 
  cerr << error_msg << endl;

  if ( DEBUG )
  {
    cerr << strerror(errno) << endl;
  }
  
  // close socket to server
  close( client_sock_fd ); 
  exit(EXIT_FAILURE);
}


void cleanup()
{
  cout << "Client has closed" << endl;
  if ( close( client_sock_fd ) < 0 )
  {
    die ( "Failed to close" );
  }
  exit(EXIT_SUCCESS);
}


void sig_wrap_cleanup( int sig )
{
  // wrapper for sigaction to pass int to sig which is never used
  cleanup();
}


void handle_input()
{
  string inp = "";
  cin >> inp;
  int bytes_sent = -1;
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

  bytes_sent = write( client_sock_fd, (inp.substr(0,3)).c_str(), 3 );
  if ( bytes_sent < 0 )
  {
    die( "Unable to send message." );
  }
  if ( bytes_sent == 0 )
  {
    cout << "No bytes sent." << endl;
  }
  if ( bytes_sent > 0 )
  {
    cout << "Sent " << inp << " to server." << endl;
  }
}


void handle_server_msg()
{
  int pos_found = 0;
  string msg = "";

  pos_found = past_data_read.find( TERM_STR );
  msg = past_data_read.substr( 0, pos_found );
  past_data_read = past_data_read.substr( pos_found + 2 );

  switch( msg.front() )
  {
    case EXIT:
      cout << msg.substr( 1 ) << endl;
      cleanup();
      break;

    case MESSAGE:
      cout << "Message Recieved: " << endl;
      cout << msg.substr( 1 ) << endl; 
      break;

    case CARDS:
      if ( DEBUG )
      {
        cout << "Recieved card with bitcode: " << msg.substr( 1 ) << endl;
      }
      break;

    default:
      break;
  }

  if ( ! past_data_read.empty() )
  {
    handle_server_msg ();
  }
}


void read_server_msg()
{
  int bytes_read = 0;
  char buffer[1024];

  bytes_read = read( client_sock_fd, &buffer, 1023);
  if ( bytes_read != -1 )
  {
    past_data_read.append( buffer );
    handle_server_msg();
  }
  else
  {
    die("Something went wrong. Likely that server was closed.");
  }
}


void game_loop ()
{
  struct pollfd poll_fds[] = {{STDIN_FILENO, POLLIN, 0}, {client_sock_fd, POLLIN, 0}};

  for ( ;; )
  {
    if ( poll( poll_fds, 2, 1 ) == -1 )
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
        read_server_msg();
      } 
    }
  }
}


void init_client( int port, char *addr )
{
  struct sockaddr_in server_addr = {};
  struct hostent *server;

  client_sock_fd = socket( AF_INET, SOCK_STREAM, 0 );
  if ( client_sock_fd < 0 )
  {
    die("Failed to create socket.");
  }

  server = gethostbyname( addr );
  if ( server == nullptr )
  {
    die("No such server.");
  }

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons( port );
  // convert ip string to binary 
  if ( inet_pton( AF_INET, addr, &server_addr.sin_addr ) != 1 )
  {
    die("Can't convert ip.");
  }

  if ( connect( client_sock_fd, (struct sockaddr *) &server_addr, sizeof server_addr) == -1 )
  {
    die("Cannot connect to server.");
  }

  // bind TERM to cleanup
  struct sigaction action = {};

  action.sa_handler = sig_wrap_cleanup;
  sigaction( SIGINT, &action, nullptr );
}


int main( int argc, char *argv[] )
{

  if ( argc == 1 || argc > 3 )
  {
    die("Usage: ./client <port>");
  }
  else
  {
    if ( argc == 2 )
    {
      init_client( atoi( argv[1] ), LOCALHOST );
    }
    else
    {
      init_client( atoi( argv[1] ), argv[2] );
    }  
    
    game_loop();
  }

  cleanup();
}
