#include "dropboxServer.h"

void upload(int socket, Client* client){
  char buffer[BUFFER_SIZE]; // 1 KB buffer
  char path[MAXNAME * 3 + 1];
  char client_folder[MAXNAME*3];

  strcpy(buffer, S_NAME);
  write_to_socket(socket, buffer);

  read_from_socket(socket, buffer); // recebe nome do arquivo
  DEBUG_PRINT("Nome recebido: %s\n", buffer);

  char filename[MAXNAME];
  strcpy(filename, buffer);

  sprintf(client_folder, "%s/%s", serverInfo.folder, client->userid);
  sprintf(path, "%s/%s", client_folder, filename);

  strcpy(buffer, S_MODTIME);
  write_to_socket(socket, buffer); // envia "timestamp"
  read_from_socket(socket, buffer); // le o timestamp
  time_t last_modified = getTime(buffer);
  DEBUG_PRINT("MT do arquivo: %s.\n", buffer);

  int index = getFileIndex(filename, client->file_info);

  pthread_mutex_lock(&client->mutex_files[index]);
  receive_file(path, socket);
  setModTime(path, last_modified);

  client->n_files = get_dir_file_info(client_folder, client->file_info);
  // TODO: alterar essa função para apenas incrementar n_files e adicionar para client.file_info o novo valor
  // ao invés de refazer o cálculo para todo diretório.
  pthread_mutex_unlock(&client->mutex_files[index]);

  // envia o arquivo para o possível cliente conectado no outro dispositivo
  if(client->devices[0] == socket){ // envia para o dispositivo 1
    if(client->devices[1] != -1) {
      sync_device_upload(client->devices[1], filename);
    }
  } else if(client->devices[1] == socket){ // envia para o dispositivo 0
    if(client->devices[0] != -1) {
      sync_device_upload(client->devices[0], filename);
    }
  }
}

void download(int socket, Client* client){
  char buffer[BUFFER_SIZE]; // 1 KB buffer
  char filename[3*MAXNAME+1];

  strcpy(buffer, S_NAME);
  write_to_socket(socket, buffer);

  read_from_socket(socket, buffer);

  sprintf(filename, "%s/%s/%s", serverInfo.folder, client->userid, buffer);

  int index = getFileIndex(buffer, client->file_info);
  pthread_mutex_lock(&client->mutex_files[index]);
  send_file(filename, socket);
  pthread_mutex_unlock(&client->mutex_files[index]);
}

void list_server(int socket, Client* client){
  char buffer[BUFFER_SIZE]; // 1 KB buffer

  sprintf(buffer, "%d", client->n_files);
  DEBUG_PRINT("number files: %d\n", atoi(buffer));
  write_to_socket(socket, buffer);

  for(int i = 0; i < client->n_files; i++) {
    sprintf(buffer, "%s%s%s \t- MT: %s%s%s",
    COLOR_GREEN, client->file_info[i].name, COLOR_RESET,
    COLOR_YELLOW, client->file_info[i].last_modified, COLOR_RESET);
    write_to_socket(socket, buffer);
  }
}

void delete(int socket, Client* client){
  char buffer[BUFFER_SIZE]; // 1 KB buffer
  char filename[MAXNAME];
  char path[3*MAXNAME+1];
  char client_folder[MAXNAME*3];

  strcpy(buffer, S_NAME);
  write_to_socket(socket, buffer); // envia "name"

  read_from_socket(socket, buffer);
  strcpy(filename, buffer);

  sprintf(client_folder, "%s/%s", serverInfo.folder, client->userid);
  sprintf(path, "%s/%s", client_folder, filename);
  //DEBUG_PRINT("deletando arquivo %s\n", filename);

  int index = getFileIndex(filename, client->file_info);
  pthread_mutex_lock(&client->mutex_files[index]);

  if(fileExists(path)) {
  	if(remove(path) != 0) {
  		DEBUG_PRINT("Erro ao deletar o arquivo %s\n", path);
  	} else {
    	DEBUG_PRINT("Arquivo %s excluido!\n", path);
    }

    client->n_files = get_dir_file_info(client_folder, client->file_info);
    // TODO: alterar essa função para apenas decrementar n_files e remover de file_info o elemento
    // ao invés de refazer o cálculo para todo diretório.

    strcpy(buffer, S_RPL_DELETE);
    write_to_socket(socket, buffer); // envia "deletado"
  }

  pthread_mutex_unlock(&client->mutex_files[index]);

  if(client->devices[0] == socket){ // envia para o dispositivo 1
    if(client->devices[1] != -1){
      sync_device_delete(client->devices[1], filename);
    }
  } else if(client->devices[1] == socket){ // envia para o dispositivo 0
      if(client->devices[0] != -1){
        sync_device_delete(client->devices[0], filename);
      }
  }

}

void sync_device_upload(int socket, char* filename){ //TODO

}

void sync_device_delete(int socket, char* filename){ // TODO

}


void sync_dir(int socket, Client* client) {
  synchronize_client(socket, client);
}

void select_commands(int socket, char buffer[], Client* client){
  if(strcmp(buffer, S_UPLOAD) == 0){
    upload(socket, client);
  } else if(strcmp(buffer, S_DOWNLOAD) == 0){
    download(socket, client);
  } else if(strcmp(buffer, S_LS) == 0){
    list_server(socket, client);
  } else if(strcmp(buffer, S_REQ_DELETE) == 0){
    delete(socket, client);
  } else if(strcmp(buffer, S_GETSYNCDIR) == 0) {
    sync_dir(socket, client);
  }
}

int is_valid_command(char* command) {
	return
		strncmp(command, S_DOWNLOAD, strlen(S_DOWNLOAD)) == 0 ||
		strncmp(command, S_UPLOAD, strlen(S_UPLOAD)) == 0 ||
		strncmp(command, S_LS, strlen(S_LS)) == 0 ||
    strncmp(command, S_REQ_DELETE, strlen(S_REQ_DELETE)) == 0 ||
    strncmp(command, S_REQ_DC, strlen(S_REQ_DC)) == 0 ||
		strncmp(command, S_SYNC, strlen(S_SYNC)) == 0 ||
		strncmp(command, S_GETSYNCDIR, strlen(S_GETSYNCDIR)) == 0;
}
