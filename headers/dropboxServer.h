#define MAXNAME 32
#define MAXFILES 15

struct file_info
{
  char name[MAXNAME];
  char extension[MAXNAME];
  char last_modified[MAXNAME];
  int size;
};

struct client
{
  int devices[2];
  char userid[MAXNAME];
  struct file_info file_info[MAXFILES];
  int logged_in;
};


void sync_server();

void receive_file(char *file);

void send_file(char *file);
