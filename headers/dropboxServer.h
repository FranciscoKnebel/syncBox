#ifndef DROPBOXSERVER_H
#define DROPBOXSERVER_H
#include "dropboxUtil.h"
#include "user.h"
#include <semaphore.h> 

/* CONSTANTS */
#define DEFAULT_PORT 3000
#define DEFAULT_ADDRESS "127.0.0.1"
#define MAX_CLIENTS 10
#define SOCKET_BACKLOG 1
#define DEFAULT_USER_METADATA_FILE "banco_de_usuarios"

/* ERROR CODES */
#define ERROR_ON_BIND -9
#define ERROR_CREATING_SERVER_FOLDER -10
#define ERROR_CREATING_USER_FOLDER -11
#define ERROR_RECV -12
#define ERROR_ON_OPENING_SOCKET -13

typedef struct client
{
  int devices[2];
  char userid[MAXNAME];
  FileInfo file_info[MAXFILES];
  int n_files;
  int logged_in;
  sem_t exclusive;
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

void sync_server();

void receive_file(char *file);

void send_file(char *file);

void startServer();

void* continueClientProcess(Connection* connection);

int searchClient(Client* client, char* userId);

int newClient(char* userid);

int addDevice(Client* client);

int removeDevice(Client* client, int device);

void check_login_status(Client* client);

#endif