#include "dropboxServer.h"

void upload(int socket, Client* client){
  char buffer[BUFFER_SIZE]; // 1 KB buffer
  char path[MAXNAME * 3 + 1];
  char client_folder[MAXNAME*3];
  int status = 0;

  strcpy(buffer, S_NAME);
  status = write(socket, buffer, BUFFER_SIZE);
  if (status < 0) {
  	DEBUG_PRINT("ERROR writing to socket\n");
  }

  status = read(socket, buffer, BUFFER_SIZE);
  if (status < 0) {
  	DEBUG_PRINT("ERROR reading from socket\n");
  }

  char filename[MAXNAME];
  getLastStringElement(path, buffer, "/");
  strcpy(filename, path);

  sprintf(client_folder, "%s/%s", serverInfo.folder, client->userid);
  sprintf(path, "%s/%s", client_folder, filename);

  receive_file(path, socket);
}

void download(int socket, Client* client){
  char buffer[BUFFER_SIZE]; // 1 KB buffer
  char filename[3*MAXNAME+1];
  int status = 0;

  strcpy(buffer, S_NAME);
  status = write(socket, buffer, BUFFER_SIZE);
  if (status < 0) {
  	DEBUG_PRINT("ERROR writing to socket\n");
  }

  status = read(socket, buffer, BUFFER_SIZE);
  if (status < 0) {
  	DEBUG_PRINT("ERROR reading from socket\n");
  }

  sprintf(filename, "%s/%s/%s", serverInfo.folder, client->userid, buffer);
  send_file(filename, socket);
}

void list_server(int socket, Client* client){
  char buffer[BUFFER_SIZE]; // 1 KB buffer
  int status = 0;

  sprintf(buffer, "%d", client->n_files);
  DEBUG_PRINT("number files: %d\n", atoi(buffer));
  status = write(socket, buffer, BUFFER_SIZE);
  if (status < 0) {
	   DEBUG_PRINT("ERROR writing to socket\n");
  }

  for(int i = 0; i < client->n_files; i++) {
    sprintf(buffer, "%s%s%s \t- MT: %s%s%s",
    COLOR_GREEN, client->file_info[i].name, COLOR_RESET,
    COLOR_YELLOW, client->file_info[i].last_modified, COLOR_RESET);
    status = write(socket, buffer, BUFFER_SIZE);
    if (status < 0) {
    	DEBUG_PRINT("ERROR writing to socket\n");
    }
  }
}

void delete(int socket, Client* client){
  char buffer[BUFFER_SIZE]; // 1 KB buffer
  char filename[3*MAXNAME+1];
  int status = 0;

  strcpy(buffer, S_NAME);
  status = write(socket, buffer, BUFFER_SIZE);
  if (status < 0) {
  	DEBUG_PRINT("ERROR writing to socket\n");
  }

  status = read(socket, buffer, BUFFER_SIZE);
  if (status < 0) {
  	DEBUG_PRINT("ERROR reading from socket\n");
  }

  getLastStringElement(filename, buffer, "/");
  char file[3*MAXNAME+1];
  strcpy(file, filename);

  sprintf(filename, "%s/%s/%s", serverInfo.folder, client->userid, file);
  //DEBUG_PRINT("deletando arquivo %s\n", filename);

  if(fileExists(filename)) {
  	if(remove(filename) != 0) {
  		DEBUG_PRINT("Erro ao deletar o arquivo %s\n", filename);
  	} else {
    	DEBUG_PRINT("Arquivo %s excluido!\n", filename);
    }

    strcpy(buffer, S_RPL_DELETE);
    status = write(socket, buffer, BUFFER_SIZE);
    if (status < 0) {
      DEBUG_PRINT("ERROR writing to socket\n");
    }
  }
}

void select_commands(int socket, char buffer[], Client* client){
  if(strcmp(buffer, S_UPLOAD) == 0){
    DEBUG_PRINT("\nupload\n");
    upload(socket, client);
  } else if(strcmp(buffer, S_DOWNLOAD) == 0){
    DEBUG_PRINT("\ndownload\n");
    download(socket, client);
  } else if(strcmp(buffer, S_LS) == 0){
    DEBUG_PRINT("\nlist_server\n");
    list_server(socket, client);
  } else if(strcmp(buffer, S_REQ_DELETE) == 0){
    DEBUG_PRINT("\ndelete\n");
    delete(socket, client);
  }
}
