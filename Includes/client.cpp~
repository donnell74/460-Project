#include "client.h"

// Start of Client
void Client::die ( string error_msg )
{
  // standardize error_msg so no extra new lines
  error_msg.erase( error_msg.find_last_not_of(" \n\r\t")+1 ); 
  cerr << error_msg;

  if ( DEBUG )
  {
    cerr << strerror( errno );
  }
  
  // close socket to server
  if ( close( client_sock_fd ) == -1 )
  {
    cerr << strerror( errno );
  }
  exit(EXIT_FAILURE);
}


void Client::cleanup()
{
  cout << "You have been disconnected.";
  send_message( "QUI" );
  exit(EXIT_SUCCESS);
}


Client::Client( int port, char *addr, char * user )
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

  send_message( user );
}


void Client::read_server_msg()
{
  int bytes_read = 0;
  char buffer[1024] = {};

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


Client::~Client()
{
  cleanup(); 
}


void Client::wait_for_input ()
{
  struct pollfd poll_fds[] = {{STDIN_FILENO, POLLIN, 0}, {client_sock_fd, POLLIN, 0}};

  for ( ;; )
  {
    if ( poll( poll_fds, 2, 100 ) == -1 )
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


void Client::send_message( string msg )
{
  int bytes_sent = write( client_sock_fd, msg.c_str(), msg.size() );
  if ( bytes_sent < 0 )
  {
    die( "Unable to send message." );
  }
  if ( bytes_sent == 0 )
  { 
    //cout << "No bytes sent." << endl;
  }
  if ( bytes_sent > 0 )
  {  
    //cout << "Sent " << msg << " to server." << endl;
  }
}


string Client::get_next_msg ()
{
  int pos_found = past_data_read.find( TERM_STR );
  string msg = past_data_read.substr( 0, pos_found );
  past_data_read = past_data_read.substr( pos_found + 2 );
  return msg;
}

string Client::get_past_data_read ()
{
  return past_data_read; 
}


void Client::set_past_data_read ( string setVal )
{
  past_data_read = setVal; 
}
