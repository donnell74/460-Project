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
      
#endif /* NETWORKING_H */
