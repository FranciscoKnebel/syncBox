// COMMANDS
#define COMMAND_UPLOAD "upload"
#define COMMAND_DOWNLOAD "download"
#define COMMAND_LS "list_server"
#define COMMAND_LC "list_client"
#define COMMAND_SYNC "get_sync_dir"
#define COMMAND_HELP "help"
#define COMMAND_CREDITS "credits"
#define COMMAND_CLEAR "clear"
#define COMMAND_EXIT "exit"

void command_upload(char* path);
void command_download(char* path);
void command_listserver();
void command_listclient();
void command_getsyncdir();
void command_credits();
void command_clear();
void command_help();
