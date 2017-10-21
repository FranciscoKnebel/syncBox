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

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

/* Interface */
int cprintf(char* ansicode, char* string);

/* Files */
int getFilesize(FILE* pFile);
int fileExists_stat(char* pathname, struct stat* st);
int fileExists(char* pathname);

/* System */
char* getUserName();
char* getUserHome();
