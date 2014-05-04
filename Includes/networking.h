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
#include "cardlib.h"
#define NETWORKING_H
#define DEBUG 1
#define MAXCLIENTS 25
#define DEFAULT_PORT 51717
#define TERM_STR "XX"

#define ALPHA 1 
#define LQWERTY 2 
#define RQWERTY 3 
#define COLEMAK 4 

using namespace std;

/* structs */
struct Client_t
{
  int sock_fd;  
  sockaddr_in addr;
  string name;
  int score;
  int keyboard;
  bool on_streak;
};

#endif /* NETWORKING_H */
