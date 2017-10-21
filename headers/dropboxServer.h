#include "dropboxUtil.h"

typedef struct client
{
  int devices[2];
  char userid[MAXNAME];
  struct file_info file_info[MAXFILES];
  int logged_in;
} Client;

void sync_server();

void receive_file(char *file);

void send_file(char *file);

void startServer();
