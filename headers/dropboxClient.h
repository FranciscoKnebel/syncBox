#include "dropboxUtil.h"
#include "client/commands.h"
#include "client/help.h"

// COMMANDS
#define COMMAND_UPLOAD "upload"
#define COMMAND_DOWNLOAD "download"
#define COMMAND_LS "list_server"
#define COMMAND_LC "list_client"
#define COMMAND_SYNC "get_sync_dir"
#define COMMAND_HELP "help"
#define COMMAND_CREDITS "credits"
#define COMMAND_EXIT "exit"

// RETURN CODES
#define MAXNAMESIZE_REACHED -1

  // interface
#define COMMAND_WITH_ARGUMENTS 0
#define COMMAND_WITH_NO_ARGUMENTS 1
#define NO_ARGUMENT_PROVIDED -2


/* STRUCTURES */
struct user_info {
  char id[MAXNAME];
  char folder[MAXNAME*2];
};
extern struct user_info user;

int connect_server(char *host, int port);

void sync_client();

void send_file(char *file);

void get_file(char *file);

void delete_file(char *file);

void close_connection();

void show_client_interface();
