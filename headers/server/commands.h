void select_commands(int socket, char buffer[], Client* client);
void upload(int socket, Client* client);
void download(int socket, Client* client);
void list_server(int socket, Client* client);
void delete(int socket, Client* client);
void sync_dir(int socket, Client* client);
void sync_device_upload(int socket, char* filename);
void sync_device_delete(int socket, char* filename);

int is_valid_command(char* command);
