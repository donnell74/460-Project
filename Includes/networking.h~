#include<iostream>
#include <sys/socket.h>
#ifndef NETWORKING_H
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <poll.h>
#define NETWORKING_H

using namespace std;

/* structs - move to header file later */
struct Client
{
  int sock_fd;  
  sockaddr_in addr;
};

/* Constants */
int DEBUG = 1;
int DEFAULT_PORT = 51717;
char LOCALHOST[] = "127.0.0.1";
int MAXCLIENTS = 25;

#endif /* NETWORKING_H */
