#include "server.h"


//|die
void Server::die( string error_msg )
{
    // standardize error_msg so no extra new lines
    error_msg.erase( error_msg.find_last_not_of( " \n\r\t" ) + 1 ); 
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
    exit( EXIT_FAILURE );
}


//|Server
Server::Server( int port, char *addr, int delay_time )
{
    poll_fds = { { STDIN_FILENO, POLLIN, 0 } };
    delay = delay_time;
    //time( &start );
    
    last_correct = 0;
    streak = 0;

    // create socket for server
    server_sock_fd = socket( AF_INET, SOCK_STREAM, 0 );
    if ( server_sock_fd < 0 )
    {
        die( "Error opening socket. " );
    }

    // add server socket to poll_fds
    struct pollfd server_pollfd = { server_sock_fd, POLLIN, 0 };
    poll_fds.push_back( server_pollfd );

    // create sock addr we use for the rest of program
    struct sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons( port );

    // convert ip text string to binary
    if ( inet_pton( AF_INET, addr, &server_addr.sin_addr ) != 1 )
    {
        die( "Error converting addr during server initialization. " );
    }  

    int yes = 1;
  
    // lose the pesky "Address already in use" error message
    if ( setsockopt( server_sock_fd, SOL_SOCKET, SO_REUSEADDR, 
         &yes, sizeof( int ) ) == -1 ) 
    {
        die( "setsockopt" );
    } 
  
    // bind socket to the server ip
    if ( bind( server_sock_fd, 
             ( struct sockaddr *) &server_addr,
               sizeof server_addr ) < 0 )
    {
        die( "Can't bind to server socket. " );
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

//|~Server
Server::~Server()
{
    cleanup();
}


//|get_client_list
vector<Client_t> Server::get_client_list()
{
    return client_list;
}


//|force_game_over
void Server::force_game_over()
{
    cout << "Attempting a forced game over" << endl;
    deck->remove_all_cards();
}


//|sendMessage
void Server::sendMessage( int sock_fd, char type, string msg )
{
    //makes calls to sendMessage more readable
    msg.insert( msg.begin(), type );
 
    msg.append( TERM_STR ); 
    int bytes_sent = write( sock_fd, msg.c_str(), msg.size() ); 
    if ( bytes_sent < 0 )
    {
        die( "Unable to send message." );
    }
}


// Sends desired amount of cards to all clients.
//|send_playing_cards
void Server::send_playing_cards( vector<int>indexes )
{
  
    string cards_to_send = create_playing_cards( indexes, deck, 
                                                 playing_deck );
    for ( auto client_it : get_client_list() )
    {
        sendMessage( client_it.sock_fd, 'c', cards_to_send );
    }

}


//|send_null_cards
void Server::send_null_cards( vector<int>indexes )
{
    string cards_to_send = "";
    for ( unsigned int i = 0; i < indexes.size(); i++ )
    {
        cards_to_send += ( char )255;
        cards_to_send += ":" + to_string( indexes[i] ) + ";";
    }

    for ( auto client_it : get_client_list() )
    {
        sendMessage( client_it.sock_fd, 'c', cards_to_send );
    }
}


//|update_scores
void Server::update_scores()
{
    string unames_string = "";

    for ( unsigned int i = 0; i < client_list.size(); i++ )
    {
        unames_string += client_list[i].name;
        unames_string += " ";
        unames_string += to_string( client_list[i].score );
        unames_string += "<>";
    }

    for ( unsigned int j = 0; j < client_list.size(); j++ )
    {
        sendMessage( client_list[j].sock_fd, 'u', unames_string ); 
    }
}


//Preliminary game tasks
//|begin_game
void Server::begin_game()
{
    if ( delay >= 0 )
    {
        delay = -1;
        getitimer( ITIMER_REAL, &delay_timer );
        delay_timer.it_value.tv_sec = 0;
        delay_timer.it_value.tv_usec = 0;
        setitimer( ITIMER_REAL, &delay_timer, nullptr );
        send_playing_cards( std_indexes );
        update_scores();
        display_sets( playing_deck->get_cards() );
    }
    else
    {
        cout << "Game has already been started." << endl;
    }
}


//|start_timer
void Server::start_timer()
{
    delay_timer = { {1, 0}, {1, 0} };
    if ( setitimer( ITIMER_REAL, &delay_timer, nullptr ) == -1 ) 
    {
        cerr << strerror( errno ) << ": can't set interval timer\n";
    }
}


//|disable_timer
void Server::disable_timer()
{
    //Disable itimer
    getitimer( ITIMER_REAL, &delay_timer );
    delay_timer.it_value.tv_sec = 0;
    delay_timer.it_value.tv_usec = 0;
    setitimer( ITIMER_REAL, &delay_timer, nullptr );
    update_client_timer();
    begin_game();
}


//|check timer
void Server::check_timer()
{
    if ( delay == 0 )
    {
        disable_timer();
    }
    
    delay -= 1;
}


//|update_client_timer
void Server::update_client_timer()
{ 
    for ( unsigned int i = 0; i < client_list.size(); i++ )
    {
        sendMessage( client_list[ i ].sock_fd, 't', to_string( delay ) ); 
    }
}


//Checks client guess
//|check_guess
int Server::check_guess( char* guess, Deck* deck, Deck* playing_deck, 
                         vector<char> accepted_keys )
{
    switch( guess[0] )
    {
        case 'n':
            if ( num_sets( playing_deck->get_cards() ) == 0 )
            {
                return 0;
            }
            return 1;

        default:
        {
            //Set to check
            vector<Card*>cset;
  
            int idx_1 = map_card( toupper( guess[0]), accepted_keys );
            int idx_2 = map_card( toupper( guess[1]), accepted_keys );
            int idx_3 = map_card( toupper( guess[2]), accepted_keys );
            if ( playing_deck->get_card( idx_1 ) == nullptr ||
                 playing_deck->get_card( idx_2 ) == nullptr ||
		 playing_deck->get_card( idx_3 ) == nullptr )
            {
               return 3;
            } 
            cset.push_back( playing_deck->get_card( idx_1 ) );
            cset.push_back( playing_deck->get_card( idx_2 ) );
            cset.push_back( playing_deck->get_card( idx_3 ) );

            //Display set info -- Testing purposes
            //display_card( cset[0] );
            //display_card( cset[1] );
            //display_card( cset[2] );       

            //Actual set checking
            if ( check_set ( cset ) )
            {
                return 2;
            }
            return 3;    
        }
    }
}


//|cleanup
void Server::cleanup ()
{
    cout << "\nServer going down ( as well as all clients )." << endl;
    // client list is empty or we quit while creating 
    //first client connection

    if ( ! client_list.empty() && ( client_list.front().sock_fd != 0 ) )
    {
        for ( auto client_it : client_list )
        {
            cout << "Closing client " << client_it.sock_fd << endl;
            sendMessage( client_it.sock_fd, 
                         'x', "You have been disconnected" ); 
        }
    }
  
    cout << "All clients should be closed." << endl;
    cout << "Now closing server." << endl;

    if ( close( server_sock_fd ) == -1  && DEBUG )
    {
        cerr << strerror( errno ) << endl;
    }  

    cout << "Server should now be closed." << endl;
    exit( EXIT_SUCCESS );
}


//|update_score
void Server::update_score( int client_sock_fd, int guess_type )
{
    int index = 0;

    for ( unsigned int i = 0; i <= client_list.size(); i++ )
    {
        if ( client_list[i].sock_fd == client_sock_fd )
        {
            index = i;
            sendMessage( client_sock_fd, 'm', "Updating your score" );
            break;
        }
    }

    //pthread_mutex_lock( &mutex );
    switch ( guess_type )
    {   
        case 0:

            //Correct Set Guess
            client_list[index].score += 10;

            //Check to see if client gets bonus
            if ( client_sock_fd == last_correct && 
                 client_list[index].on_streak )
            {
                if ( streak > 14 )
                {
                    client_list[index].score += FIB[14];
                }
                else
                {
                    client_list[index].score += FIB[streak];
                }

            streak += 1;
            }

            //Client is not on streak
            else
            {
                client_list[last_correct].on_streak = false;
                last_correct = client_sock_fd;
                streak = 0;
                client_list[index].on_streak = true;
            }
            break;

        case 1:

            //Incorrect 'No Set' Guess
            client_list[index].score += -5;
            client_list[index].on_streak = false;
            break;

        case 2:

            //Correct 'No Set' Guess
            client_list[index].score += 5;

            //Check to see if client in on streak
            if ( client_sock_fd == last_correct &&
                 client_list[index].on_streak )
            {
                if ( streak > 14 )
                {
                    client_list[index].score += FIB[14];
                }
                else
                {
                    client_list[index].score += FIB[streak];
                }
            streak += 1;
            }
            
            //Client is not on streak
            else
            {
                streak = 0;
                client_list[last_correct].on_streak = false;
                last_correct = client_sock_fd;
                client_list[index].on_streak = true;
            }
            break;
        
        case 3:

            //Incorrect Set Guess
            client_list[index].score += -3;
            client_list[index].on_streak = false;

        default:
            break;
    }
    //pthread_mutex_unlock( &mutex );
    update_scores();
}


//|check_name
string Server::check_name( string buffer )
{
    int count = 0;
    string mybuffer = buffer;
    for ( auto it : usernames )
    { 
	if ( it == mybuffer )
	{
	    mybuffer = buffer + to_string( count );
	    ++count;
	    it = usernames.front();
	}
    }

    usernames.push_back( mybuffer );
    return mybuffer;
}


//|wait_for_cleanup
void Server::wait_for_client()
{
    for ( ;; )
    {
        char buffer[14] = { 0 }; // used for client name

        // emplace might be replacing, check when implementing multiple client
        Client_t this_client = {};
        socklen_t client_len = sizeof ( this_client.addr );
        this_client.sock_fd = accept( server_sock_fd,
                                    ( struct sockaddr * ) &this_client.addr,
                                      &client_len );

        if ( this_client.sock_fd < 0 )
        {
            die( "Error on accept. " );
        }

        else
        {   
	    
	    if ( delay > 0 && client_list.size() <= 12 )
	    {
	        cout << "Connected new client." << endl;

	        // add poll fd and send message
	        struct pollfd client_sock_fd = { this_client.sock_fd, 
                                                 POLLIN, 0 };

	        // CRITICAL SECTION
	        pthread_mutex_lock( &mutex );
                this_client.score = 0;
                this_client.on_streak = false;
                // Read name
	        int hr = read( this_client.sock_fd, &buffer, 15 );
                if ( hr <= 0 )
                {
                    cout << strerror( errno ) << endl;
                } 

                else
                {
                    char second_buffer[14] = {0};
                    strncpy( second_buffer, &buffer[1], 14 ); 
                    this_client.name = check_name( second_buffer );
                    this_client.keyboard = buffer[0] - '0';
                }
	        client_list.push_back( this_client );
	        pthread_mutex_unlock( &mutex );
         
                // tell client what server knows
	        sendMessage( this_client.sock_fd, 'n', 
                             this_client.name );
	        sendMessage( this_client.sock_fd, 'k', 
                             to_string(this_client.keyboard) );
                sendMessage( this_client.sock_fd, 'm',
                             "Make a guess " ); 
	        poll_fds.push_back( client_sock_fd );
	        //send_playing_cards( std_indexes );
	        //display_sets ( playing_deck->get_cards() );	
	    }
            else
	    {    
                sendMessage( this_client.sock_fd, 'm', 
                             "The server is closed." ); 
                disconnect_client( this_client.sock_fd ); 
	    }   
        }
    }
}


//|disconnect_client
void Server::disconnect_client( int client_sock_fd )
{

    for ( auto client_it = client_list.begin(); 
          client_it != client_list.end(); ++client_it )
    {
        if ( client_it->sock_fd == client_sock_fd )
        {
            client_list.erase( client_it );
            break;
         }
    }

    for ( auto poll_fd_it = poll_fds.begin(); 
          poll_fd_it != poll_fds.end(); ++poll_fd_it )
    {
        if ( poll_fd_it->fd == client_sock_fd )
        {
            poll_fds.erase( poll_fd_it );
            break;
        }
    }

    // even if close fails, server acts if client has died
    // as of point
    sendMessage( client_sock_fd, 'x', "You have been disconnected" ); 
    cout << "Client with fd of " << client_sock_fd 
         << " has disconnected." << endl;
}


//|compareByScore
bool compareByScore( const Client_t &a, const Client_t &b )
{
    return a.score > b.score;
}


//|score_sort
void Server::score_sort()
{

    pthread_mutex_lock( &mutex );
    sort( client_list.begin(), client_list.end(), compareByScore );
    pthread_mutex_unlock( &mutex );
}


//|respond_to_client
void Server::respond_to_client ( int client_sock_fd, char* guess )
{

    sendMessage( client_sock_fd, 'm', "Checking guess.." ); 
    vector<Client_t>::iterator client;
    for ( auto client_it = client_list.begin(); 
          client_it != client_list.end(); ++client_it )
    {
        if ( client_it->sock_fd == client_sock_fd )
        {
            client = client_it;
        }
    }

    int guess_type = check_guess( guess, deck, playing_deck, 
                                  ACCEPTED_CHARS[client->keyboard - 1] );
    //Update Score
    pthread_mutex_lock( &mutex );
    update_score( client_sock_fd, guess_type );
    pthread_mutex_unlock( &mutex );

    switch( guess_type )
    {
        case 0:
            //Correct No Set Guess
            sendMessage( client_sock_fd, 'm', "Correct!" );

            if ( !deck->empty( 0 ) )
            {
	      
                int limit = 11;
                vector<int>indxs = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
              
                for ( int i = 0; i < 6; i++ )
                {
                    indxs.erase( indxs.begin() + rand( limit ) );
                    limit--;
                }

                for ( int j = 0; j < 6; j++ )
                {
                    deck->add_card( playing_deck->get_card( indxs[j] ) );
                }

	
                deck->shuffle();
                send_playing_cards( indxs );
	
            }
            display_sets( playing_deck->get_cards() );
            break;

        case 1:
            //Incorrect No Set
            sendMessage( client_sock_fd, 'm', "Incorrect" );
            break;


        case 2:
            //Correct Set Guess
            sendMessage( client_sock_fd, 'm', "Correct!" );

            if ( !deck->empty( 0 ) )
	    { 
                vector<int>indexes;
                indexes.push_back( map_card( toupper ( guess[0] ), 
                                   ACCEPTED_CHARS[client->keyboard - 1] ) );
                indexes.push_back( map_card( toupper ( guess[1] ), 
                                   ACCEPTED_CHARS[client->keyboard - 1] ) );
                indexes.push_back( map_card( toupper ( guess[2] ), 
                                   ACCEPTED_CHARS[client->keyboard - 1] ) );
                playing_deck->remove_card( indexes[0] );
                playing_deck->remove_card( indexes[1] );
                playing_deck->remove_card( indexes[2] );
                send_playing_cards( indexes );
                display_sets( playing_deck->get_cards() );
                break;
	    }
            else
            {
                vector<int>indexes;
                indexes.push_back( map_card( toupper ( guess[0]), 
                                   ACCEPTED_CHARS[client->keyboard - 1]  ) );
                indexes.push_back( map_card( toupper ( guess[1]), 
                                   ACCEPTED_CHARS[client->keyboard - 1]  ) );
                indexes.push_back( map_card( toupper ( guess[2]), 
                                   ACCEPTED_CHARS[client->keyboard - 1]  ) );
                playing_deck->remove_card( indexes[0] );
                playing_deck->remove_card( indexes[1] );
                playing_deck->remove_card( indexes[2] );
                send_null_cards( indexes );
                break;
            }
            break;
				     
        case 3:
            //Incorrect Set Guess
            sendMessage( client_sock_fd, 'm', "Not a set" );
            break;

        default:
            break;
    }

    //|Game over
    if ( deck->empty( 0 ) && num_sets( playing_deck->get_cards() ) == 0 )
         // && playing_deck->count( 1 ) < 13 )  
    {
        cout << "Sending game over to all clients" << endl;

        score_sort();
        string unames_string = "";

        for ( unsigned int i = 0; i < client_list.size(); i++ )
        {
            unames_string += client_list[i].name;
            unames_string += " ";
            unames_string += to_string( client_list[i].score );
            unames_string += "<>";
        }

        for ( auto client_it = client_list.begin(); 
              client_it != client_list.end(); ++client_it )
        {
          sendMessage( client_it->sock_fd, 'o', unames_string );
        }

        sleep( 15 );
        //Perform end game tasks(disconnect clients, etc.)
        cleanup();
    }
}


//|receive_input
void Server::receive_input( int client_sock_fd )
{
    int bytes_read = 0;
    char buffer[4] = { 0 }; // will only care about first 3 if guess

    bytes_read = read( client_sock_fd, &buffer, 3 );
    if ( bytes_read > 0)
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
            respond_to_client( client_sock_fd, buffer );
        }
    }
    else
    {
      disconnect_client( client_sock_fd );
    }  
}


//|wait_for_input
void Server::wait_for_input()
{
    for ( ;; )
    {
        // &poll_fds[0] can cause problems on reallocation
        if ( poll( &poll_fds[0], poll_fds.size(), 100 ) == -1 )
        {
	    if (errno == EINTR )
	    {
	        continue;
	    }
	    die( "Problem with input." );
        }    

	//No need for Else statement, die terminates
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
// End of Server

