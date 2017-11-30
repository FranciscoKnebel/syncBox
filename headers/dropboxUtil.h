#define DEBUG // Comentar esta linha para desativar DEBUG mode.

#include "includes.h"
#include "communication.h"

// syncBox CONSTANTS
#define MAXNAME 64
#define MAXPATH 256
#define MAXFILES 128

#define DIRECTORY_FILE 0;
#define REGULAR_FILE 1;

#define BUFFER_SIZE 1024

// ERROR CODES
#define FILE_NOT_FOUND -2

typedef struct file_info
{
  char name[MAXNAME];
  char extension[MAXNAME];
  char last_modified[MAXNAME];
  int size;
} FileInfo;

typedef struct d_file
{
  char path[MAXNAME];
  char name[MAXNAME];
} DFile;

typedef struct dir_content {
  char* path;
  struct d_file* files;
  int* counter;
} DirContent;

int get_dir_content(char * path, struct d_file files[], int* counter);
int get_dir_file_info(char * path, FileInfo files[]);
int get_all_entries(char * path, struct d_file files[]);
int print_dir_content(char * path);
int getFileModifiedTime(char *path, char* last_modified);
void getFileExtension(const char *filename, char* extension);
int  getFileSize(char *path);
void getLastStringElement(char filename[], char* string, const char *separator);
time_t getTime(char* last_modified);
int setModTime(char* path, time_t mod_time);
int older_file(char* last_modified_file_1, char* last_modified_file_2);

/* I/O */
int readToFile(FILE* pFile, int file_size, int sockid);
void write_to_socket(int socket, char* buffer);
void read_from_socket(int socket, char* buffer);
/* Interface */
int cprintf(char* ansicode, char* string);

#ifdef DEBUG
  #define DEBUG_PRINT(fmt, args...) fprintf(stderr, "DEBUG:\t"fmt, ## args)
#else
  #define DEBUG_PRINT(fmt, args...) /* Don't do anything in release builds */
#endif

/* Files */
int getFilesize(FILE* pFile);
int fileExists_stat(char* pathname, struct stat* st);
int fileExists(char* pathname);
int getFileIndex(char* filename, FileInfo file_info[]);

/* System */
char* getUserName();
char* getUserHome();
