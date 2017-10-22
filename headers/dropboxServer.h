#include "dropboxUtil.h"

/* CONSTANTS */
#define DEFAULT_PORT 5100
#define DEFAULT_ADDRESS "127.0.0.1"

/* ERROR CODES */
#define ERROR_ON_BIND -9
#define ERROR_CREATING_SERVER_FOLDER -10
#define ERROR_CREATING_USER_FOLDER -11

typedef struct client
{
  int devices[2];
  char userid[MAXNAME];
  FileInfo file_info[MAXFILES];
  int n_files;
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
