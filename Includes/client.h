#ifndef CLIENT_H
#define CLIENT_H
#include "networking.h"
#include "cardlib.h"

/* Function required by user to implement  */
void handle_server_msg ();
void handle_input();
void quit_options( bool );

class Client
{
  private:
    /* Globals */
    int client_sock_fd;
    string past_data_read;

  public:
    Client( int, char *, char * );
    ~Client();
    void die( string );
    void cleanup();
    void read_server_msg();
    void wait_for_input();
    void send_message( string );
    string get_past_data_read();
    void set_past_data_read( string );
    string get_next_msg();
};

#endif
