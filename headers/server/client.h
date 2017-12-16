#include "dropboxServer.h"

ClientList* newClient(char* userid, SSL* socket, ClientList* client_list);
Client* searchClient(char* userId, ClientList* client_list);
ClientList* removeClient(Client* client, ClientList* client_list);

int addDevice(Client* client, SSL* socket);
int removeDevice(Client* client, int device, ClientList* client_list);

ClientList* check_login_status(Client* client, ClientList* client_list);
