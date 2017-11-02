#include "dropboxServer.h"

void syncronize_client(int sockid_sync, Client* client_sync) {
  char buffer[BUFFER_SIZE]; // 1 KB buffer
  int status = 0;

  DEBUG_PRINT("Iniciando sincronização do cliente...\n");

  status = read(sockid_sync, buffer, BUFFER_SIZE);
  if (status < 0) {
    DEBUG_PRINT("ERROR reading from socket\n");
  }

  DEBUG_PRINT("COMMAND: %s\n", buffer);
  if(strcmp(buffer, S_SYNC) == 0) {
    DEBUG_PRINT("sincronizar!\n");
  }

  sprintf(buffer, "%d", client_sync->n_files);
  DEBUG_PRINT("Client number of files: %d.\n", client_sync->n_files);
  status = write(sockid_sync, buffer, BUFFER_SIZE);
  if (status < 0) {
    DEBUG_PRINT("ERROR writing to socket\n");
  }

  for(int i = 0; i < client_sync->n_files; i++) {
    strcpy(buffer, client_sync->file_info[i].name);
    status = write(sockid_sync, buffer, BUFFER_SIZE);
    if (status < 0) {
      DEBUG_PRINT("ERROR writing to socket\n");
    }
    strcpy(buffer, client_sync->file_info[i].last_modified);

    status = write(sockid_sync, buffer, BUFFER_SIZE);
    if (status < 0) {
      DEBUG_PRINT("ERROR writing to socket\n");
    }

    status = read(sockid_sync, buffer, BUFFER_SIZE);
    if (status < 0) {
      DEBUG_PRINT("ERROR reading from socket\n");
    }

    if(strcmp(buffer, S_DOWNLOAD) == 0){
      download(sockid_sync, client_sync);
    }
  }
}

void syncronize_server(int sockid_sync, Client* client_sync) {
  char buffer[BUFFER_SIZE]; // 1 KB buffer
  char path[MAXNAME * 3 + 1];
  char last_modified[MAXNAME];
  char file_name[MAXNAME];
  int  status = 0;
  int  number_files_client = 0;

  DEBUG_PRINT("Iniciando sincronização do servidor...\n");

  status = read(sockid_sync, buffer, BUFFER_SIZE);
  if (status < 0) {
    DEBUG_PRINT("ERROR reading from socket\n");
  }
  number_files_client = atoi(buffer);
  DEBUG_PRINT("Number files client: %d\n", number_files_client);

  char last_modified_file_2[MAXNAME];
  for(int i = 0; i < number_files_client; i++){
    status = read(sockid_sync, buffer, BUFFER_SIZE);
    if (status < 0) {
    	DEBUG_PRINT("ERROR reading from socket\n");
    }
    strcpy(file_name, buffer);
    //printf("%s\n", file_name);
    status = read(sockid_sync, buffer, BUFFER_SIZE);
    if (status < 0) {
      DEBUG_PRINT("ERROR reading from socket\n");
    }
    strcpy(last_modified, buffer);
    //printf("ultima modificaçao server: %s\n", last_modified);
    sprintf(path, "%s/%s/%s", serverInfo.folder, client_sync->userid, file_name);
    getFileModifiedTime(path, last_modified_file_2);
    //printf("ultima modificacao user: %s\n", last_modified_file_2);
    if(!fileExists(path) || older_file(last_modified, last_modified_file_2) == 1) {
      strcpy(buffer, S_GET);
      status = write(sockid_sync, buffer, BUFFER_SIZE);
      if (status < 0) {
        DEBUG_PRINT("ERROR writing to socket\n");
      }

      status = read(sockid_sync, buffer, BUFFER_SIZE);
      if (status < 0) {
        DEBUG_PRINT("ERROR reading from socket\n");
      }

      if(strcmp(buffer, S_UPLOAD) == 0) {
        upload(sockid_sync, client_sync);
      }
  	} else {
  		strcpy(buffer, S_OK);
  		status = write(sockid_sync, buffer, BUFFER_SIZE);
  	}
  }
}
