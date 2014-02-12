#ifndef NETWORKING_H
#include <iostream>
#include <string>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <string.h>
#include <netdb.h>
#define NETWORKING_H
#define DEBUG 1
#define MAXCLIENTS 25
#define DEFAULT_PORT 51717
#define TERM_STR "XX"

using namespace std;




/* structs */
struct Client_t
{
  int sock_fd;  
  sockaddr_in addr;
};

/* Function required by user to implement  */
void handle_input();

class Server
{
  private:
    pthread_mutex_t mutex;
    vector<Client_t> client_list;
    vector<pollfd> poll_fds;

    int server_sock_fd;

  public:
    Server( int, char * );
    ~Server();
    void die ( string );
    vector<Client_t> get_client_list ( );
    void sendMessage( int, char , string );
    void cleanup ();
    void wait_for_client ( );
    void disconnect_client( int );
    void recieve_input( int );
    void wait_for_input ();
};
      
/* Function required by user to implement  */
void handle_server_msg ();

class Client
{
  private:
    /* Globals */
    int client_sock_fd;
    string past_data_read;

  public:
    Client( int, char * );
    ~Client();
    void die ( string );
    void cleanup();
    void read_server_msg();
    void wait_for_input ();
    void send_message( string );
    string get_past_data_read();
    void set_past_data_read( string );
    string get_next_msg ();
};

#endif /* NETWORKING_H */
