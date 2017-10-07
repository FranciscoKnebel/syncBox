#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define RECEIVE_REQUEST_PORT 2345;
#define SEND_REQUEST_PORT 3456;
#define MAX_CONNECTIONS 5;

struct client {
  int devices[2];
  char userid[MAXNAME];
  struct file_info[MAXFILES];
  int logged_in;
}

struct file_info {
  char name[MAXNAME];
  char extension[MAXNAME];
  char last_modified[MAXNAME];
  int size;
}
