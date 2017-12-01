#ifndef util_h
#define util_h
  #include "dropboxUtil.h"
#endif

int read_to_file(FILE* pFile, int file_size, int sockid);
int write_to_socket(int socket, char* buffer);
int read_from_socket(int socket, char* buffer);
