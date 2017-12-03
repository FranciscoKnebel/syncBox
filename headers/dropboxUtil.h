//#define DEBUG // Comentar esta linha para desativar DEBUG mode.

// syncBox CONSTANTS
#define MAXNAME 128
#define MAXPATH 256
#define MAXFILES 128

#define DIRECTORY_FILE 0;
#define REGULAR_FILE 1;

#define BUFFER_SIZE 1024

// ERROR CODES
#define FILE_NOT_FOUND -2

#include "includes.h"
#include "communication.h"

#ifndef util_modules_h
#define util_modules_h
  #include "util/types.h"
  #include "util/files.h"
  #include "util/io.h"
#endif

/* Interface */
int cprintf(char* ansicode, char* string);

#ifdef DEBUG
  #define DEBUG_PRINT(fmt, args...) fprintf(stderr, "DEBUG:\t"fmt, ## args)
  #define DEBUG_PRINT_COND(cond, fmt, args...) if(cond) fprintf(stderr, "DEBUG:\t"fmt, ## args)
#else
  #define DEBUG_PRINT(fmt, args...) /* Don't do anything in release builds */
  #define DEBUG_PRINT_COND(fmt, args...)
#endif

/* System */
char* getUserName();
char* getUserHome();
