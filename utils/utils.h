#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include <fcntl.h>

#define BACKLOG 10
#define ERROR -1 

typedef struct sockaddr_in Addr_in;
typedef struct sockaddl Addr;

int Init_Address(const char *ip, const char *port, bool server);
bool load_config(const char *config_path, char *server_ip, char *server_port) ;

#endif