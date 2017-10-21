#include "dropboxUtil.h"

typedef struct client
{
  int devices[2];
  char userid[MAXNAME];
  FileInfo file_info[MAXFILES];
  int logged_in;
} Client;

typedef struct server_info {
  char ip[MAXNAME];
  char folder[MAXNAME*2];
  int port;
} ServerInfo;

extern ServerInfo serverInfo;

void sync_server();

void receive_file(char *file);

void send_file(char *file);

void startServer();
