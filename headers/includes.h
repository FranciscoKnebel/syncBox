#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <utime.h>
#include <locale.h>

#include <dirent.h>
#include <unistd.h>
#include <pthread.h>
#include <pwd.h>
#include <semaphore.h>

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
