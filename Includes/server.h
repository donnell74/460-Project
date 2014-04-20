#ifndef SERVER_H
#define SERVER_H
#include "networking.h"
#include "cardlib.h"
#include <time.h>

/* Function required by user to implement  */
void handle_input();


class Server
{
  private:
    pthread_mutex_t mutex;
    vector<Client_t> client_list;
    vector<pollfd> poll_fds;
    int server_sock_fd;
    vector<string> usernames;
    int delay;
    time_t start;
    time_t end;
    int last_correct;
    int streak;

  public:
    Deck* deck;
    Deck* playing_deck;
    Server( int, char *, int );
    ~Server();
    void die( string );
    vector<Client_t> get_client_list();
    void sendMessage( int, char , string );
    void send_playing_cards( vector<int> );
    void send_null_cards( vector<int> );
    int check_guess( char*, Deck*, Deck* );
    void cleanup();
    void update_score( int, int );
    void update_scores();
    void wait_for_client();
    void disconnect_client( int );
    void receive_input( int );
    void respond_to_client( int, char* );
    void wait_for_input();
    string check_name( string );
};
      
#endif /* SERVER_H */
