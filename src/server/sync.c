#include "dropboxServer.h"

void synchronize_client(int sockid_sync, Client* client_sync) { // executa primeiro
  char buffer[BUFFER_SIZE]; // 1 KB buffer

  DEBUG_PRINT("Iniciando sincronização do cliente.\n");

  read_from_socket(sockid_sync, buffer);

  DEBUG_PRINT("COMMAND: %s\n", buffer);
  if(strcmp(buffer, S_SYNC) == 0) {
    DEBUG_PRINT("sincronizar!\n");
  }

  sprintf(buffer, "%d", client_sync->n_files);
  DEBUG_PRINT("Client number of files: %s.\n", buffer);
  write_to_socket(sockid_sync, buffer); // numero de arquivos

  for(int i = 0; i < client_sync->n_files; i++) {
    strcpy(buffer, client_sync->file_info[i].name);
    DEBUG_PRINT("Nome do arquivo a enviar: %s\n", buffer);
    write_to_socket(sockid_sync, buffer);

    strcpy(buffer, client_sync->file_info[i].last_modified);
    DEBUG_PRINT("enviando Last modified: %s\n", buffer);
    write_to_socket(sockid_sync, buffer);

    read_from_socket(sockid_sync, buffer);
    DEBUG_PRINT("Recebido: %s\n", buffer);

    if(strcmp(buffer, S_DOWNLOAD) == 0) { // se recebeu S_DOWNLOAD do buffer, faz o download
      download(sockid_sync, client_sync);
    }
  }

  DEBUG_PRINT("Encerrando sincronização do cliente.\n");
}

void synchronize_server(int sockid_sync, Client* client_sync) {
  char buffer[BUFFER_SIZE]; // 1 KB buffer
  char path[MAXNAME * 3 + 1];
  char last_modified[MAXNAME];
  char file_name[MAXNAME];
  int  number_files_client = 0;

  DEBUG_PRINT("Iniciando sincronização do servidor.\n");

  read_from_socket(sockid_sync, buffer);
  number_files_client = atoi(buffer);
  DEBUG_PRINT("Number files client: %d\n", number_files_client);

  char last_modified_file_2[MAXNAME];
  for(int i = 0; i < number_files_client; i++){
    read_from_socket(sockid_sync, buffer); // nome do arquivo
    strcpy(file_name, buffer);
    DEBUG_PRINT("Nome recebido: %s\n", file_name);

    read_from_socket(sockid_sync, buffer); // last modified
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
      write_to_socket(sockid_sync, buffer);

      read_from_socket(sockid_sync, buffer); // resposta do cliente
      DEBUG_PRINT("Resposta recebida: %s\n", buffer);

      if(strcmp(buffer, S_UPLOAD) == 0) {
        upload(sockid_sync, client_sync);
      }
  	} else {
      DEBUG_PRINT("Upload desnecessário do arquivo %s.\n\n", file_name);
  		strcpy(buffer, S_OK);
  		write_to_socket(sockid_sync, buffer); // envia "ok"
  	}
  }

  DEBUG_PRINT("Encerrando sincronização do servidor.\n");
}
