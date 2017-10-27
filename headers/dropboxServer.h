#ifndef util_h
#define util_h
  #include "dropboxUtil.h"
#endif

#ifndef server_aux_h
#define server_aux_h
  #include "server/client.h"
  #include "server/commands.h"
#endif

#ifndef server_h
#define server_h

/* CONSTANTS */
#define DEFAULT_PORT 3000
#define DEFAULT_ADDRESS "127.0.0.1"
#define MAX_CLIENTS 10
#define SOCKET_BACKLOG 1

/* ERROR CODES */
#define ERROR_ON_BIND -9
#define ERROR_CREATING_SERVER_FOLDER -10
#define ERROR_CREATING_USER_FOLDER -11
#define ERROR_RECV -12

#define CLIENT_NOTFOUND -1
#define MAX_CLIENTS_REACHED -20

typedef struct client
{
  int devices[2];
  char userid[MAXNAME];
  FileInfo file_info[MAXFILES];
  int n_files;
  int logged_in;
} Client;

typedef struct server_info {
  char ip[sizeof(DEFAULT_ADDRESS) + 2];
  char folder[MAXNAME * 2];
  int port;
} ServerInfo;

typedef struct connection_info{
  int socket_id;
  char* ip;
} Connection;

extern ServerInfo serverInfo;
extern Client clients[MAX_CLIENTS];

void sync_server();

void receive_file(char *file);

void send_file(char *file);

#endif
