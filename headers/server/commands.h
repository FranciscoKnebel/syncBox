void select_commands(SSL *socket, char buffer[], Client* client);
void upload(SSL *socket, Client* client);
void download(SSL *socket, Client* client);
void list_server(SSL *socket, Client* client);
void delete(SSL *socket, Client* client);
void sync_dir(SSL *socket, Client* client);
void sync_local(SSL *socket, Client* client);

int is_valid_command(char* command);
