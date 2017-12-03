#include "dropboxClient.h"

void synchronize_local(int sockid, int print) { // executa primeiro
  FileInfo localFiles[MAXFILES];
  int number_files_client = 0;
  char path[MAXNAME * 3 + 1];
  char buffer[BUFFER_SIZE];
  char file_name[MAXNAME];
  char last_modified[MAXNAME];
  int number_files_server = 0;

  number_files_client = get_dir_file_info(user.folder, localFiles); // pega os arquivos do cliente

  int files_to_delete[number_files_client];
  for(int i = 0; i < number_files_client; i++){
    files_to_delete[i] = 0;
  }

  if(print){
    DEBUG_PRINT("Iniciando sincronização local.\n");
  }

  strcpy(buffer, S_SYNC);
	write_to_socket(sockid, buffer); // envia comando "sync"

	read_from_socket(sockid, buffer); // recebe numero de arquivos no server

	number_files_server = atoi(buffer);
  if(print){
	   DEBUG_PRINT("'%s%d%s' arquivos no servidor\n", COLOR_GREEN, number_files_server, COLOR_RESET);
  }

	char last_modified_2[MAXNAME];
	for(int i = 0; i < number_files_server; i++) {
		read_from_socket(sockid, buffer); // nome do arquivo no server
		strcpy(file_name, buffer);
    if(print){
      DEBUG_PRINT("\n");
      DEBUG_PRINT("%d: Nome recebido: %s\n", i, file_name);
    }
    for(int i = 0; i < number_files_client; i++){
      if(strcmp(file_name, localFiles[i].name) == 0){
        files_to_delete[i] = 1;
      }
    }

		read_from_socket(sockid, buffer); // timestamp
		strcpy(last_modified, buffer);
    if(print){
      DEBUG_PRINT("%d: Last modified recebido: %s\n", i, last_modified);
    }
		sprintf(path, "%s/%s", user.folder, file_name);
    if(print){
      DEBUG_PRINT("%d: Path: %s\n", i, path);
    }

		if(!fileExists(path)) {
      if(print){
        DEBUG_PRINT("%d: Arquivo %s não existe... baixando\n", i, file_name);
      }
	    get_file(file_name, NULL);
		} else {
      if(print){
        DEBUG_PRINT("%d: Arquivo existe, verificando se necessita baixar...\n",  i);
      }
      getFileModifiedTime(path, last_modified_2);
      if(print){
        DEBUG_PRINT("%d: Last modified local: %s\n", i, last_modified_2);
      }
      if (older_file(last_modified, last_modified_2) == 1) {
        if(print){
          DEBUG_PRINT("%d: Arquivo %s mais velho... baixando\n", i, file_name);
        }
        get_file(file_name, NULL);
      } else {
        if(print){
          DEBUG_PRINT("%d: Download desnecessário do arquivo %s.\n", i, file_name);
        }
  			strcpy(buffer, S_OK);
  			write_to_socket(sockid, buffer);
  		}
    }
	}

  for(int i = 0; i < number_files_client; i++){
    if(files_to_delete[i] == 0){
      sprintf(path, "%s/%s", user.folder, localFiles[i].name);
      if(remove(path) == 0){
        DEBUG_PRINT("Arquivo %s deletado!\n", path);
      }
    }
  }
  if(print){
    DEBUG_PRINT("Encerrando sincronização local.\n\n");
  }
}

void synchronize_server(int sockid) {
  FileInfo localFiles[MAXFILES];
  char path[MAXNAME * 3 + 1];
  char buffer[BUFFER_SIZE];
  int number_files_client = 0;

  DEBUG_PRINT("Iniciando sincronização do servidor.\n");

  number_files_client = get_dir_file_info(user.folder, localFiles);
	sprintf(buffer, "%d", number_files_client);
	write_to_socket(sockid, buffer); // envia numero de arquivos do cliente pro server

	for(int i = 0; i < number_files_client; i++) {
		strcpy(buffer, localFiles[i].name);
    DEBUG_PRINT("\n");
    DEBUG_PRINT("%d: Nome enviado: %s\n", i, localFiles[i].name);
		write_to_socket(sockid, buffer);
		strcpy(buffer, localFiles[i].last_modified);
    DEBUG_PRINT("%d: Last modified enviado: %s\n", i, localFiles[i].last_modified);
		write_to_socket(sockid, buffer);

		read_from_socket(sockid, buffer); // resposta do server

    DEBUG_PRINT("%d: Recebido: %s\n", i, buffer);
		if(strcmp(buffer, S_GET) == 0) {
			sprintf(path, "%s/%s", user.folder, localFiles[i].name);
			send_file(path, FALSE);
      DEBUG_PRINT("%d: Enviando arquivo %s\n", i, path);
		} else if (strcmp(buffer, S_OK) == 0) {
      DEBUG_PRINT("%d: Envio desnecessário.\n", i);
    }
	}

  DEBUG_PRINT("Encerrando sincronização do servidor.\n");
}
