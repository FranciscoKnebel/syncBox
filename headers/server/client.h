#include "dropboxServer.h"

void clearClients();

int newClient(char* userid, int socket);
int searchClient(Client* client, char* userId);

int addDevice(Client* client, int socket);
int removeDevice(Client* client, int device);

void check_login_status(Client* client);
