#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define MAX_CONNECTIONS 5
#define MAXNAME 64
#define MAXFILES 128
#define PORT 3222

#define SYNC_REQUEST "sync"
#define RECEIVE_REQUEST "receive-request"
#define SEND_REQUEST "send-request"

struct file_info {
  char name[MAXNAME];
  char extension[MAXNAME];
  char last_modified[MAXNAME];
  int size;
};

struct client {
  int devices[2];
  char userid[MAXNAME];
  struct file_info file[MAXFILES];
  int logged_in;
};

int receive_connection(void);
