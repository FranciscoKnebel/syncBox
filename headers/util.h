#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <dirent.h>
#include <unistd.h>
#include <pthread.h>
#include <pwd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/inotify.h>

//sockets
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#define TRUE 1
#define FALSE 0

#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_RESET   "\x1b[0m"

/* Interface */
int cprintf(char* ansicode, char* string);

#ifdef DEBUG
  #define DEBUG_PRINT(fmt, args...) fprintf(stderr, fmt, ## args)
#else
  #define DEBUG_PRINT(fmt, args...) /* Don't do anything in release builds */
#endif

/* Files */
int getFilesize(FILE* pFile);
int fileExists_stat(char* pathname, struct stat* st);
int fileExists(char* pathname);

/* System */
char* getUserName();
char* getUserHome();
