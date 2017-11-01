
#ifndef USER_H
#define USER_H
#include "tree.h"
#include "dropboxServer.h"
#include "fileUtil.h"

int save_clients(char file_name[]);
int get_clients_from_file( char *file_name);
struct client *get_client( char *client_name);
int add_client( char *client_name);

void startSem();
int add_file_to_client(struct client *c,char* file_name);
int remove_file_from_client(struct client *c, char* file_name);
int get_file_from_client(struct client *c, char* file_name,char * buffer);

#endif
