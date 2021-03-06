#ifndef util_h
#define util_h
  #include "dropboxUtil.h"
#endif

#ifndef server_aux_h
#define server_aux_h
  #include "server/client.h"
  #include "server/commands.h"
  #include "server/sync.h"
#endif

#ifndef server_h
#define server_h

/* CONSTANTS */
#define DEFAULT_PORT 3000
#define DEFAULT_ADDRESS "127.0.0.1"
#define MAX_CLIENTS 10
#define SERVER_FOLDER "syncBox_users"

/* ERROR CODES */
#define ERROR_ON_BIND -9
#define ERROR_CREATING_SERVER_FOLDER -10
#define ERROR_CREATING_USER_FOLDER -11
#define ERROR_RECV -12

#define CLIENT_NOTFOUND -1
#define MAX_CLIENTS_REACHED -20

typedef struct client
{
  SSL* devices[2];
  char userid[MAXNAME];
  FileInfo file_info[MAXFILES];
  pthread_mutex_t mutex_files[MAXFILES];
  int n_files;
  int logged_in;
  char host[MAXNAME];
  int port;
} Client;

typedef struct client_list
{
  Client* client;
  struct client_list *next;
} ClientList;

typedef struct server_info {
  char ip[sizeof(DEFAULT_ADDRESS) * 2];
  char folder[MAXNAME * 2];
  int port;
} ServerInfo;

typedef struct connection_info{
  int socket_id;
  SSL *ssl;
  char* ip;
  int porta;
} Connection;

extern ServerInfo serverInfo;

pthread_mutex_t mutex_exclusao_mutua_lock;

void sync_server(SSL *sockid_sync, Client* client_sync);

void receive_file(char *file, SSL *sockid_upload);

void send_file(char *file, SSL *sockid_download, int send_mod_time);

int updateReplicas(char* file_path, char* command);

int reconnect_server_replica();

#endif
