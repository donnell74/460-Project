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


/* Globals */
int server_sock_fd = 0;
vector<Client> client_list;


void error ( string error_msg )
{
  // standardize error_msg so no extra new lines
  error_msg.erase(error_msg.find_last_not_of(" \n\r\t")+1); 
  cerr << error_msg << endl;

  if ( DEBUG )
  {
    cerr << strerror(errno) << endl;
  }
  
  // close socket to server
  close( server_sock_fd ); 
  exit(EXIT_FAILURE);
}

void init_server ( int port, char *addr )
{
  cout << "Entering init_server." << endl;
  // create socket for server
  server_sock_fd = socket( AF_INET, SOCK_STREAM, 0 );
  if ( server_sock_fd < 0 )
  {
    error("Error opening socket. ");
  }
 
  // create sock addr we use for the rest of program
  struct sockaddr_in server_addr = {};
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons( port );

  // convert ip text string to binary
  if ( inet_pton( AF_INET, addr, &server_addr.sin_addr ) != 1 )
  {
    error("Error converting addr during server initialization. ");
  }  

  // bind socket to the server ip
  if ( bind( server_sock_fd, 
             (struct sockaddr *) &server_addr,
             sizeof server_addr ) < 0)
  {
    error("Can't bind to server socket. ");
  }

  // start listening for a connection
  // change SOMAXCON to a int if requirements
  // require a limit on clients
  if ( listen( server_sock_fd, SOMAXCONN ) < 0 )
  {
    error("Unable to listen.");
  }

  // output that it is listening
  cout << "Listening on port " << addr << endl;
}


void sendMessage( int sock_fd, string msg )
{
  int bytes_sent = write( sock_fd, msg.c_str(), msg.size()); 
  if ( bytes_sent < 0 )
  {
    error("Unable to send message.");
  }
}


void wait_for_client ()
{
  // will need to be edited for mulithreading
  // currently just connects one client
  // insert at begining because it doesn't matter position of client in 
  // list but placing in front is faster
  auto client_it = client_list.emplace( client_list.begin(), Client{});
  // emplace might be replacing, check when implementing multiple client
  char buffer[256];
  socklen_t client_len = sizeof (client_it->addr);
  client_it->sock_fd = accept(server_sock_fd,
                                  (struct sockaddr *) &client_it->addr,
                                  &client_len );

  if ( client_it->sock_fd < 0 )
  {
    error("Error on accept. ");
  }
  else
  {
    cout << "Connected new client." << endl;
    memset( buffer, 0, sizeof buffer );
    sendMessage( client_it->sock_fd, "You have been connected." );
  }
}


void cleanup ()
{
  close( server_sock_fd );
  exit(EXIT_SUCCESS);
}


int main ( int argc, char *argv[] )
{

  if ( argc == 1 || argc > 3 )
  {
    error("Usage: ./server <port>");
  }
  else
  {
    if ( argc == 2 ) // No port number given
    {
      init_server( atoi( argv[1] ), LOCALHOST );  
    }
    
    if ( argc == 3 ) // Addr and Port given
    {
      init_server( atoi( argv[1] ), argv[2] );
    }
  }
 
  wait_for_client(); 
  cleanup(); 
}
