#ifndef SERVER_H
#define SERVER_H
#include "networking.h"
#include "cardlib.h"
#include <time.h>
#include <algorithm>

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
    const int FIB [ 15 ] =
    { 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987, 1597 };

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
    int check_guess( char*, Deck*, Deck*, vector<char> );
    void cleanup();
    void update_score( int, int );
    void insertion_sort();
    void update_scores();
    void wait_for_client();
    void disconnect_client( int );
    void receive_input( int );
    void respond_to_client( int, char* );
    void score_sort();
    void wait_for_input();
    void force_game_over();
    string check_name( string );
};
      
#endif /* SERVER_H */
