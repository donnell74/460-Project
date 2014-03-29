#include "networking.h"


/* Constants */

void Server::die ( string error_msg )
{
  // standardize error_msg so no extra new lines
  error_msg.erase(error_msg.find_last_not_of(" \n\r\t")+1); 
  cerr << error_msg << endl;

  if ( DEBUG )
  {
    cerr << strerror( errno ) << endl;
  }
  
  // close socket to server
  if ( server_sock_fd )
  {
    if ( close( server_sock_fd ) == -1 && DEBUG )
    {
      cerr << strerror( errno ) << endl;
    }
  }
  exit(EXIT_FAILURE);
}


Server::Server ( int port, char *addr )
{
  poll_fds = {{STDIN_FILENO, POLLIN, 0}};

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

}


Server::~Server()
{
  cleanup();
}


vector<Client_t> Server::get_client_list ( )
{
  return client_list;
}


void Server::sendMessage( int sock_fd, char type, string msg )
{
  msg.insert(msg.begin(), type); // makes calls to sendMessage more readable
  msg.append( TERM_STR ); 
  int bytes_sent = write( sock_fd, msg.c_str(), msg.size()); 
  if ( bytes_sent < 0 )
  {
    die( "Unable to send message." );
  }
}


void Server::cleanup ()
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



void Server::wait_for_client ( )
{
  for (;;)
  {
    // will need to be edited for mulithreading
    // currently just connects one client
    // auto client_it = client_list.push_back( client_list.begin(), Client{});
    // emplace might be replacing, check when implementing multiple client
    Client_t this_client = {};
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
    }
  }

}


void Server::disconnect_client( int client_sock_fd )
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
  // as of point
  sendMessage( client_sock_fd, 'x', "You have been disconnected"); 
  cout << "Client with fd of " << client_sock_fd << " has disconnected." << endl;
}


void Server::recieve_input( int client_sock_fd )
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


void Server::wait_for_input ()
{
  for ( ;; )
  {
    // &poll_fds[0] can cause problems on reallocation
    if ( poll( &poll_fds[0], poll_fds.size(), 100 ) == -1 )
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
// End of Server

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
  cout << "You have been disconnected." << endl;
  send_message( "QUI" );
  exit(EXIT_SUCCESS);
}


Client::Client( int port, char *addr )
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
