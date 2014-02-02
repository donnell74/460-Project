/* cpp includes */
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <poll.h>
#include <netdb.h>
using namespace std;

/* Constants */
int DEBUG = 1;
int DEFAULT_PORT = 51717;
char LOCALHOST[] = "127.0.0.1";
const char EXIT = 'x';
const char MESSAGE = 'm';

/* Globals */
int client_sock_fd = 0;
char buffer[256]; // global so not redefined every handle event

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
  close( client_sock_fd );
  exit(EXIT_SUCCESS);
}


void handle_input()
{
  string inp = {};
  cin >> inp;
  int bytes_sent = -1;

  for ( auto it = inp.begin(); it != inp.end(); ++it )
  {
    *it = toupper(*it);
  } 

  // substr will only get first 3, stop long string entries
  inp = inp.substr(0, 3);

  bytes_sent = write( client_sock_fd, inp.c_str(), inp.size() );
  if ( bytes_sent < 0 )
  {
    die( "Unable to send message." );
  }
  if ( bytes_sent == 0 )
  {
    cout << "No bytes sent." << endl;
  }
}


void handle_server_msg()
{
  char type = 0;
  int bytes_read = 0;

  bytes_read = read( client_sock_fd, &type, 1);
  if ( bytes_read != -1 )
  {
    switch( type )
    {
      case EXIT:
        bytes_read = read( client_sock_fd, &buffer, 255 );
        if ( bytes_read == -1 )
        {
          die("Lost connection with server.");
        }

        cout << buffer << endl;
        cleanup();
        break;

      case MESSAGE:
        bytes_read = read( client_sock_fd, &buffer, 255 ); 
        if ( bytes_read == -1 )
        {
          die("Lost connection with server.");
        }
        
        cout << buffer << endl; 
        break;

      default:
        break;
    }
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
    if ( poll( poll_fds, 2, -1 ) == -1 )
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
        handle_server_msg();
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
