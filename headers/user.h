#include "tree.h"
#include <sting.h>
#include "dropboxServer.h"

int * get_clients_from_file(const char *file_name);
Client *get_client(const char *client_name);
int add_client(const char *client_name);

int add_file_to_client(Client *c,const char* file_name);
int remove_file_from_client(Client *c,const char* file_name);
int get_file_from_client(Client *c,const char* file_name,char * buffer);