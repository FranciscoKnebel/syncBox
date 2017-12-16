#include "dropboxServer.h"

void synchronize_client(SSL *ssl_sync, Client* client_sync) { // executa primeiro
  char buffer[BUFFER_SIZE]; // 1 KB buffer

  DEBUG_PRINT("Iniciando sincronização do cliente.\n");

  read_from_socket(ssl_sync, buffer);

  DEBUG_PRINT("COMMAND: %s\n", buffer);
  if(strcmp(buffer, S_SYNC) == 0) {
    DEBUG_PRINT("sincronizar!\n");
  }

  sprintf(buffer, "%d", client_sync->n_files);
  DEBUG_PRINT("Client number of files: %s.\n", buffer);
  write_to_socket(ssl_sync, buffer); // numero de arquivos

  for(int i = 0; i < client_sync->n_files; i++) {
    strcpy(buffer, client_sync->file_info[i].name);
    DEBUG_PRINT("Nome do arquivo a enviar: %s\n", buffer);
    write_to_socket(ssl_sync, buffer);

    strcpy(buffer, client_sync->file_info[i].last_modified);
    DEBUG_PRINT("enviando Last modified: %s\n", buffer);
    write_to_socket(ssl_sync, buffer);

    read_from_socket(ssl_sync, buffer);
    DEBUG_PRINT("Recebido: %s\n", buffer);

    if(strcmp(buffer, S_DOWNLOAD) == 0) { // se recebeu S_DOWNLOAD do buffer, faz o download
      download(ssl_sync, client_sync);
    }
  }

  DEBUG_PRINT("Encerrando sincronização do cliente.\n");
}

void synchronize_server(SSL *ssl_sync, Client* client_sync) {
  char buffer[BUFFER_SIZE]; // 1 KB buffer
  char path[MAXNAME * 3 + 1];
  char last_modified[MAXNAME];
  char file_name[MAXNAME];
  int  number_files_client = 0;

  DEBUG_PRINT("Iniciando sincronização do servidor.\n");

  read_from_socket(ssl_sync, buffer);
  number_files_client = atoi(buffer);
  DEBUG_PRINT("Number files client: %d\n", number_files_client);

  char last_modified_file_2[MAXNAME];
  for(int i = 0; i < number_files_client; i++){
    read_from_socket(ssl_sync, buffer); // nome do arquivo
    strcpy(file_name, buffer);
    DEBUG_PRINT("Nome recebido: %s\n", file_name);

    read_from_socket(ssl_sync, buffer); // last modified
    strcpy(last_modified, buffer);
    DEBUG_PRINT("Last modified recebido: %s\n", last_modified);

    sprintf(path, "%s/%s/%s", serverInfo.folder, client_sync->userid, file_name);

    int needToUpload = FALSE;
    if(!fileExists(path)) {
      DEBUG_PRINT("Arquivo não existe, então pedindo upload para o servidor.\n");
      needToUpload = TRUE;
    } else {
      DEBUG_PRINT("Arquivo existe, então buscando o MT local para comparar.\n");
      getFileModifiedTime(path, last_modified_file_2);
      DEBUG_PRINT("Last modified local: %s\n", last_modified_file_2);

      if(older_file(last_modified, last_modified_file_2) == 1) {
        DEBUG_PRINT("Arquivo é mais velho, pedindo upload da versão mais recente.\n");
        needToUpload = TRUE;
      }
    }

    if(needToUpload) {
      DEBUG_PRINT("Arquivo precisa ser trazido do cliente. Pedindo arquivo.\n");
      strcpy(buffer, S_GET);
      write_to_socket(ssl_sync, buffer);

      read_from_socket(ssl_sync, buffer); // resposta do cliente
      DEBUG_PRINT("Resposta recebida: %s\n", buffer);

      if(strcmp(buffer, S_UPLOAD) == 0) {
        upload(ssl_sync, client_sync);
      }
  	} else {
      DEBUG_PRINT("Upload desnecessário do arquivo %s.\n\n", file_name);
  		strcpy(buffer, S_OK);
  		write_to_socket(ssl_sync, buffer); // envia "ok"
  	}
  }

  DEBUG_PRINT("Encerrando sincronização do servidor.\n");
}


void synchronize_replica_send(SSL *ssl_sync, ClientList* ClientList, char* serverFolder) { // envia informaçoes de todos os usuarios para o servidor replica

  struct d_file files[MAXFILES*MAX_CLIENTS];
  int counter = 0;
  char buffer[BUFFER_SIZE]; // 1 KB buffer
  char fullPath[MAXNAME*2];
  char last_modified[MAXNAME];

  DEBUG_PRINT("synchronize_replica_send: Iniciando sincronização do servidor replica.\n");

  read_from_socket(ssl_sync, buffer); // le S_SYNC

  DEBUG_PRINT("synchronize_replica_send: COMMAND: %s\n", buffer);
  if(strcmp(buffer, S_SYNC) == 0) {
    DEBUG_PRINT("synchronize_replica_send: sincronizar replica!\n");
  }

  get_dir_content(serverFolder, files, &counter);

  sprintf(buffer, "%d", counter);
  DEBUG_PRINT("synchronize_replica_send: Number of files to send: %s.\n", buffer);
  write_to_socket(ssl_sync, buffer); // numero de arquivos

  for (int i = 0; i < counter; i++) {
    sprintf(fullPath, "%s/%s", files[i].path, files[i].name);
    strcpy(buffer, fullPath + strlen(serverFolder) + 1);
    DEBUG_PRINT("synchronize_replica_send: Enviando: %s\n", buffer);
    write_to_socket(ssl_sync, buffer); // envia nome
    // envia arquivo
    char* name = malloc(strlen(fullPath) + 1);
    strcpy(name, fullPath);
    DEBUG_PRINT("synchronize_replica_send: Nome: %s\n", name + strlen(serverFolder) + 1);
    if (strchr(name + strlen(serverFolder) + 1, '/') != NULL){
      getFileModifiedTime(fullPath, last_modified);
      strcpy(buffer, last_modified);
      DEBUG_PRINT("synchronize_replica_send: enviando Last modified: %s\n", buffer);
      write_to_socket(ssl_sync, buffer); // envia last modified
      DEBUG_PRINT("synchronize_replica_send: Enviando: %s\n", fullPath);
      send_file(fullPath, ssl_sync, FALSE);
    } else{
      DEBUG_PRINT("synchronize_replica_send: Enviando nome da pasta!\n");
    }
  }

  DEBUG_PRINT("Encerrando sincronização da replica.\n");
}

void synchronize_replica_receive(SSL *ssl_sync, char* serverFolder) {
  int number_files_server = 0;
  char last_modified[MAXNAME];
  char filePath_server[MAXNAME*2];
  char filePath_local[MAXNAME*2];
  char filename[MAXNAME];
  int counter = 0;
  char buffer[BUFFER_SIZE];
  DEBUG_PRINT("Iniciando sincronização local da replica.\n");


  // primeiro exclui todos os arquivos no servidor
  struct d_file files[MAXFILES*MAX_CLIENTS];
  char file_to_delete [MAXNAME*2];
  get_dir_content(serverFolder, files, &counter);

  for(int i = 0; i < counter; i++){
      sprintf(file_to_delete, "%s/%s", files->path, files->name);
      if(remove(file_to_delete) == 0) {
        DEBUG_PRINT("Arquivo %s deletado!\n", file_to_delete);
      }
  }

  strcpy(buffer, S_SYNC);
	write_to_socket(ssl_sync, buffer); // envia comando "sync"

	read_from_socket(ssl_sync, buffer); // recebe numero de arquivos no server

	number_files_server = atoi(buffer);
  DEBUG_PRINT("'%s%d%s' arquivos no servidor\n", COLOR_GREEN, number_files_server, COLOR_RESET);

  DEBUG_PRINT("Pasta do servidor: %s\n", serverFolder);
	for(int i = 0; i < number_files_server; i++) {
		read_from_socket(ssl_sync, buffer); // nome do arquivo no server
		strcpy(filePath_server, buffer);
    DEBUG_PRINT("synchronize_replica_send: Path recebido: %s\n", filePath_server);
    sprintf(filename, "%s", buffer); //TODO: retirar xxx
    DEBUG_PRINT("synchronize_replica_send: Filename a receber: %s\n", filename);
    // recebe o arquivo
    sprintf(filePath_local, "%s/%s", serverFolder, filename);
    DEBUG_PRINT("synchronize_replica_send: Recebendo: %s\n", filePath_local);
    if (strchr(filename, '/') != NULL){ // se tiver '/', é arquivo, senão é pasta
      read_from_socket(ssl_sync, buffer); // timestamp
      strcpy(last_modified, buffer);
      DEBUG_PRINT("synchronize_replica_send: %d: Last modified recebido: %s\n", i, last_modified);
      receive_file(filePath_local, ssl_sync);
      time_t last_modified_time = getTime(last_modified);
      setModTime(filePath_local, last_modified_time);
    } else{
      char dir_user[MAXNAME];
      sprintf(dir_user, "%s/%s", serverFolder, filename);
      if(mkdir(dir_user, 0777) != 0) {
        printf("Error creating user folder in server '%s'.\n", filename);
      } else{
        DEBUG_PRINT("Criado pasta!\n");
      }
    }

	}
}
