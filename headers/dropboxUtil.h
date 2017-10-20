#include "util.h"

// syncBox CONSTANTS
#define MAXNAME 64
#define MAXPATH 256
#define MAXFILES 128

#define DIRECTORY_FILE 0;
#define REGULAR_FILE 1;

#define BUFFER_SIZE 1024

// ERROR CODES
#define FILE_NOT_FOUND -2

struct file_info
{
  char name[MAXNAME];
  char extension[MAXNAME];
  char last_modified[MAXNAME];
  int size;
};

struct d_file
{
  char path[MAXNAME];
  char name[MAXNAME];
};

struct dir_content {
  char* path;
  struct d_file* files;
  int* counter;
};

int get_dir_content(char * path, struct d_file files[], int* counter);
int get_all_entries(char * path, struct d_file files[]);
int print_dir_content(char * path);
