#ifndef util_h
#define util_h
  #include "dropboxUtil.h"
#endif

#ifndef client_aux_h
#define client_aux_h
  #include "client/commands.h"
  #include "client/help.h"
  #include "client/sync.h"
  #include "client/watcher.h"
#endif

#ifndef client_h
#define client_h

#define INTRO_MESSAGE "v0.0.1 - 08/11/2017\n"

// RETURN CODES
#define MAXNAMESIZE_REACHED -1

  // interface
#define COMMAND_WITH_ARGUMENTS 0
#define COMMAND_WITH_NO_ARGUMENTS 1
#define NO_ARGUMENT_PROVIDED -2


/* STRUCTURES */
typedef struct user_info {
  char id[MAXNAME];
  char folder[MAXNAME*2];
} UserInfo;

extern UserInfo user;
extern int sockid;

int connect_server(char *host, int port);

void sync_client();

void send_file(char *file, int response);

void get_file(char *file, char* alternatePath);

void delete_file(char *file);

void close_connection();

void list_server();

void show_client_interface();

#endif
