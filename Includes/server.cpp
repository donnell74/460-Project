#include "server.h"

int FIB [ 15 ] =
    { 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987, 1597 };


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


Server::Server ( int port, char *addr, int delay_time )
{
  poll_fds = {{STDIN_FILENO, POLLIN, 0}};
  delay = delay_time;
  time(&start);
  last_correct = -1;
  streak = 0;

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

  int yes = 1;
  
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

// Sends desired amount of cards to all clients.
void Server::send_playing_cards ( vector<int>indexes )
{
  
  string cards_to_send = create_playing_cards ( indexes, deck, playing_deck );
  for ( auto client_it : get_client_list() )
  {
    sendMessage( client_it.sock_fd, 'c', cards_to_send ); 
  }

}

//Checks client guess
vector<int> Server::check_guess ( char* guess, Deck* deck, Deck* playing_deck )
{
    vector<int> indexes;
  
    //Client guesses 'NOS' -- no valid sets on board
    if ( strcmp( guess, "NOS" ) == 0 && playing_deck->count( 1 ) == 12 )
    {
        if ( num_sets( playing_deck->get_cards() ) == 0)
	{
	    //Clear playing deck, reset virtual top of deck, 
            //reshuffle, and reinsert cards
	    playing_deck->clear_cards();
	    deck->reset_top();
	    deck->shuffle();
	    return std_indexes;
	}

        else 
	{
	    return indexes;
	}
    }
 
 
  //Set to check
  vector<Card*>cset;
  
  int idx_1 = map_card( guess[0] );
  int idx_2 = map_card( guess[1] );
  int idx_3 = map_card( guess[2] );

  cset.push_back( playing_deck->get_card( idx_1 ) );
  cset.push_back( playing_deck->get_card( idx_2 ) );
  cset.push_back( playing_deck->get_card( idx_3 ) );

  //Display set info -- Testing purposes
  display_card( cset[0] );
  display_card( cset[1] );
  display_card( cset[2] );
       


  if ( check_set ( cset ) )
    {
      //insert call to function that will update score
      playing_deck->remove_card( idx_1 );
      playing_deck->remove_card( idx_2 );
      playing_deck->remove_card( idx_3 );
      indexes = { idx_1, idx_2, idx_3 };
    }
    
  return indexes;
  
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


void Server::update_score( int client_sock_fd, int guess_type, char* guess )
{
    // Use fd offset instead of searching
    Client_t this_client = client_list[ client_sock_fd - 4 ];

    switch ( guess_type )
    {
        case 0:
            if ( strcmp( guess, "NOS" ) == 0 )
            {
                this_client.score += -5;
            }
            else
            {
                this_client.score += -3;
            }
            break;

        case 3:
            this_client.score += 5;
            if ( client_sock_fd == last_correct ) 
            {
                this_client.score += FIB[ streak ];
                streak += 1;
            }
            else
            {
                streak = 0;
                last_correct = client_sock_fd;
            }
            break;

        case 12:
            this_client.score += 10;
            if ( client_sock_fd == last_correct )
            {
                this_client.score += FIB[ streak ];
                streak += 1;
            }
            else
            {
                last_correct = client_sock_fd;
                streak = 0;
            }
        default: break;
    }
}


string Server::check_name ( string buffer )
{
    int count = 0;
    string mybuffer = buffer;
    for ( auto it : usernames )
    { 
	if ( it == mybuffer )
	{
	    mybuffer = buffer + to_string(count);
	    ++count;
	    it = usernames.front();
	}
    }

    return buffer;
}


void Server::wait_for_client ( )
{
  char buffer[4] = {0}; // used for client name
  
  for (;;)
  {
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
        time( &end );
	if ( difftime( end, start ) < delay &&  client_list.size() <= 12)
	{
	      cout << "Connected new client." << endl;

	      // add poll fd and send message
	      struct pollfd client_sock_fd = {this_client.sock_fd, POLLIN, 0};

	      // CRITICAL SECTION
	      pthread_mutex_lock(&mutex);
	      poll_fds.push_back(client_sock_fd);
              this_client.score = 0;
	      read( this_client.sock_fd, &buffer, 14);
	      this_client.name = check_name( buffer );
	      client_list.push_back( this_client );
	      pthread_mutex_unlock(&mutex);
	      sendMessage( this_client.sock_fd, 'm', 
              "You have been connected with username " + this_client.name + ".");
	      //send_playing_cards( std_indexes );
	      //display_sets ( playing_deck->get_cards() );	
	}
         else
	{    
              sendMessage( this_client.sock_fd, 'm', "The server is closed." ); 
              disconnect_client( this_client.sock_fd ); 
	}
    }
  }
}


void Server::disconnect_client( int client_sock_fd )
{

  // Could we just use fd offset of -4 instead of seraching?
  // like this....
  // client_list.erase( client_sock_fd - 4 );
  // and...
  // poll_fds.erase( client_sock_fd - 4 );
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


void Server::respond_to_client ( int client_sock_fd, char* guess )
{

  sendMessage( client_sock_fd, 'm', "Checking guess.." ); 
  vector<int>indexes = check_guess( guess, deck, playing_deck );
  cout << "done checking" << endl;

  // This doesn't handle incorrect No Set guesses
  switch( indexes.size() )
    {
    case 0:

      update_score( client_sock_fd, 0, guess );
      if ( strcmp( guess, "NOS" ) == 0 ) 
      {
          sendMessage( client_sock_fd, 'm', "Naw client, there's a set" );
      }
      else
      {
          sendMessage( client_sock_fd, 'm', "Naw client, that's not a set" );
      }
      //cout << client_list[ client_sock_fd - 4 ].name << endl;
      break;

    case 3:
      //1.)Send correct message to client
      last_correct = client_sock_fd;
      update_score( client_sock_fd, 3, guess );
      sendMessage( client_sock_fd, 'm',  "Correct!" );

      if ( !deck->empty( 0 ) )
	{
	  send_playing_cards( indexes );
	}
     

      //Displays current sets in playing deck -- Testing purposes only
      display_sets( playing_deck->get_cards() );
      break;
				     
    case 12:
      last_correct = client_sock_fd;
      update_score( client_sock_fd, 12, guess );
      sendMessage( client_sock_fd, 'm',  "Correct!" );
      send_playing_cards( indexes );
      //Displays current sets in playing deck -- Testing purposes only
      display_sets( playing_deck->get_cards() );
      break;
 
    default:
      break;
    }

  //Game over
  if ( deck->empty( 0 ) && num_sets( playing_deck->get_cards() ) == 0 && playing_deck->count( 1 ) < 13 )  
    {
      cout<<playing_deck->count(1)<<endl;
      sendMessage( client_sock_fd, 'm', "Game Over");
      //Perform end game tasks(disconnect clients, etc.)
      cleanup();
    }
}


void Server::receive_input( int client_sock_fd )
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
      //Respond to client's guess
      //respond_to_client( client_sock_fd, buffer );
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
            receive_input( poll_fd_it.fd );
          }
        }
      }
    }
  }
}
// End of Server

