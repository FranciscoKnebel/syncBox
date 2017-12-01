#ifndef util_h
#define util_h
  #include "dropboxUtil.h"
#endif

int readToFile(FILE* pFile, int file_size, int sockid);
void write_to_socket(int socket, char* buffer);
void read_from_socket(int socket, char* buffer);
