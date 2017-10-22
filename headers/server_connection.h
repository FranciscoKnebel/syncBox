#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include "user.h"
#include "dropboxServer.h"

#define MAX_CONNECTIONS 5
#define MAXNAME 64
#define MAXFILES 128
#define PORT 3222

#define SYNC_REQUEST "sync"
#define RECEIVE_REQUEST "receive-request"
#define SEND_REQUEST "send-request"

/*função de recepção de conexão, utiliza poll de threads para atender... para atender*/
int receive_connection(void);

/*básicamente o escalonador/coordenador, decide o que cada thread fará
a conversa do que fazer depende DO CLIENTE, que deve mandar:
SYNC_REQUEST ou
RECEIVE_REQUEST ou
SEND_REQUEST*/
void hq(int *client_socket);
