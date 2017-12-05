#ifndef util_h
#define util_h
  #include "dropboxUtil.h"
#endif

int read_to_file(FILE* pFile, int file_size, SSL* ssl);
int write_to_socket(SSL *ssl, char* buffer);
int read_from_socket(SSL *ssl, char* buffer);
