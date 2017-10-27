#include "dropboxServer.h"

void select_commands(int socket, char buffer[], Client* client){
  if(strcmp(buffer, S_UPLOAD) == 0){
    DEBUG_PRINT("\nupload\n");
    upload(socket, buffer, client);
  } else if(strcmp(buffer, S_DOWNLOAD) == 0){
    DEBUG_PRINT("\ndownload\n");
    download(socket, buffer, client);
  } else if(strcmp(buffer, S_LS) == 0){
    DEBUG_PRINT("\nlist_server\n");
    list_server(socket, buffer, client);
  }
}

void upload(int socket, char buffer[], Client* client){
  int status = 0;
  strcpy(buffer, S_NAME);
  status = write(socket, buffer, BUFFER_SIZE);

  char filename[MAXNAME];

  status = read(socket, buffer, BUFFER_SIZE);

  getLastStringElement(filename, buffer, "/");

  char file[MAXNAME];
  strcpy(file, filename);
  printf("%s\n",file);

  sprintf(filename, "%s/%s/%s", serverInfo.folder, client->userid, file);

  receive_file(filename);
}

void download(int socket, char buffer[], Client* client){
  int status = 0;
  strcpy(buffer, S_NAME);
  status = write(socket, buffer, BUFFER_SIZE);

  char filename[MAXNAME];

  status = read(socket, buffer, BUFFER_SIZE);

  strcpy(filename, buffer);
  char file[MAXNAME];
  strcpy(file, filename);
  printf("%s\n",file);

  sprintf(filename, "%s/%s/%s", serverInfo.folder, client->userid, file);

  send_file(filename);

}

void list_server(int socket, char buffer[], Client* client){
	int sockid = 4;
	int status = 0;

  sprintf(buffer, "%d", client->n_files);
	printf("number files: %d\n", atoi(buffer));
	status = write(sockid, buffer, BUFFER_SIZE);
	for(int i = 0; i < client->n_files; i++) {
		strcpy(buffer, client->file_info[i].name);
		status = write(sockid, buffer, BUFFER_SIZE);
	}
}
