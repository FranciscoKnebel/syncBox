#include "dropboxClient.h"

void synchronize_local(int sockid) { // executa primeiro
  char path[MAXNAME * 3 + 1];
  char buffer[BUFFER_SIZE];
  char file_name[MAXNAME];
  char last_modified[MAXNAME];
  int number_files_server = 0;

  int status;

  DEBUG_PRINT("Iniciando sincronização local.\n");

  strcpy(buffer, S_SYNC);
	status = write(sockid, buffer, BUFFER_SIZE); // envia comando de sincronizar
  if (status < 0) {
    DEBUG_PRINT("ERROR writing to socket\n");
  }

	status = read(sockid, buffer, BUFFER_SIZE); // recebe número de arquivos no server
  if (status < 0) {
    DEBUG_PRINT("ERROR reading from socket\n");
  }

	number_files_server = atoi(buffer);
	DEBUG_PRINT("%d arquivos no servidor\n", number_files_server);

	char last_modified_file_2[MAXNAME];
	for(int i = 0; i < number_files_server; i++) {
		status = read(sockid, buffer, BUFFER_SIZE); // recebe nome do arquivo do servidor
    if (status < 0) {
      DEBUG_PRINT("ERROR reading from socket\n");
    }
		strcpy(file_name, buffer);
    DEBUG_PRINT("Nome recebido: %s\n", file_name);
		status = read(sockid, buffer, BUFFER_SIZE); // recebe data de ultima modificacao do arquivo
    if (status < 0) {
      DEBUG_PRINT("ERROR reading from socket\n");
    }
		strcpy(last_modified, buffer);
    DEBUG_PRINT("Last modified recebido: %s\n", last_modified);

		sprintf(path, "%s/%s", user.folder, file_name);
		getFileModifiedTime(path, last_modified_file_2);
    DEBUG_PRINT("Last modified local: %s\n", last_modified_file_2);

		if(!fileExists(path)) {
		    get_file(file_name, NULL);
        DEBUG_PRINT("Arquivo %s nao existe... baixando\n", file_name);
		} else if (older_file(last_modified, last_modified_file_2) == 1) {
        get_file(file_name, NULL);
        DEBUG_PRINT("Arquivo %s mais velho... baixando\n", file_name);
    } else {
      DEBUG_PRINT("Download desnecessário do arquivo %s.\n\n", file_name);
			strcpy(buffer, S_OK);
			status = write(sockid, buffer, BUFFER_SIZE);
      if (status < 0) {
        DEBUG_PRINT("ERROR writing to socket\n");
      }
		}
	}

  DEBUG_PRINT("Encerrando sincronização local.\n");
}

void synchronize_server(int sockid) {
  FileInfo localFiles[MAXFILES];
  char path[MAXNAME * 3 + 1];
  char buffer[BUFFER_SIZE];
  int number_files_client = 0;

  int status;

  DEBUG_PRINT("Iniciando sincronização do servidor.\n");

  number_files_client = get_dir_file_info(user.folder, localFiles);
	sprintf(buffer, "%d", number_files_client);
	status = write(sockid, buffer, BUFFER_SIZE); // envia número de arquivos no cliente para o servidor
  if (status < 0) {
    DEBUG_PRINT("ERROR writing to socket\n");
  }

	for(int i = 0; i < number_files_client; i++) {
		strcpy(buffer, localFiles[i].name);
    DEBUG_PRINT("Nome enviado: %s\n", localFiles[i].name);
		status = write(sockid, buffer, BUFFER_SIZE); // envia nome do arquivo para o servidor
    if (status < 0) {
      DEBUG_PRINT("ERROR writing to socket\n");
    }
		strcpy(buffer, localFiles[i].last_modified);
    DEBUG_PRINT("Last modified enviado: %s\n", localFiles[i].last_modified);
		status = write(sockid, buffer, BUFFER_SIZE); // envia last modified para o servidor
    if (status < 0) {
      DEBUG_PRINT("ERROR writing to socket\n");
    }

    leitura:
		status = read(sockid, buffer, BUFFER_SIZE); // le resposta do servidor
    if (status < 0) {
      DEBUG_PRINT("ERROR reading from socket\n");
    }
    DEBUG_PRINT("Recebido: %s\n", buffer);
		if(strcmp(buffer, S_GET) == 0) {
			sprintf(path, "%s/%s", user.folder, localFiles[i].name);
			send_file(path, FALSE);
      DEBUG_PRINT("Enviando arquivo %s\n", path);
		} else {
      DEBUG_PRINT("NÃO RECEBEU O S_GET ESPERADO, CAGOU TUDO.\n");
      goto leitura;
    }
	}

  DEBUG_PRINT("Encerrando sincronização do servidor.\n");
}
