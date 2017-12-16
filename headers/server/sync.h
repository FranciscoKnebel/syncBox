void synchronize_client(SSL *sockid_sync, Client* client_sync);
void synchronize_server(SSL *sockid_sync, Client* client_sync);
void synchronize_replica_send(SSL *ssl_sync, ClientList* ClientList, char* serverFolder);
void synchronize_replica_receive(SSL *ssl_sync, char* serverFolder);
