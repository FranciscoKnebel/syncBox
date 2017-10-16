#include "util.h"

#include <dirent.h>

// syncBox CONSTANTS
#define MAXNAME 64
#define MAXFILES 128

// ERROR CODES
#define FILE_NOT_FOUND -2

int show_dir_content(char * path, struct dirent ** files1);
