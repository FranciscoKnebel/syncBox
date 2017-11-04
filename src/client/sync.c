#include "dropboxClient.h"

void syncronize_local(int sockid) {
  char path[MAXNAME * 3 + 1];
  char buffer[BUFFER_SIZE];
  char file_name[MAXNAME];
  char last_modified[MAXNAME];
  int number_files_server = 0;

  int status;

  DEBUG_PRINT("Iniciando sincronização local.\n");

  strcpy(buffer, S_SYNC);
	status = write(sockid, buffer, BUFFER_SIZE);
  if (status < 0) {
    DEBUG_PRINT("ERROR writing to socket\n");
  }

	status = read(sockid, buffer, BUFFER_SIZE);
  if (status < 0) {
    DEBUG_PRINT("ERROR reading from socket\n");
  }

	number_files_server = atoi(buffer);
	//DEBUG_PRINT("%d arquivos no servidor\n", number_files_server);

	char last_modified_file_2[MAXNAME];
	for(int i = 0; i < number_files_server; i++) {
		status = read(sockid, buffer, BUFFER_SIZE);
    if (status < 0) {
      DEBUG_PRINT("ERROR reading from socket\n");
    }
		strcpy(file_name, buffer);

		status = read(sockid, buffer, BUFFER_SIZE);
    if (status < 0) {
      DEBUG_PRINT("ERROR reading from socket\n");
    }
		strcpy(last_modified, buffer);

		sprintf(path, "%s/%s", user.folder, file_name);
		getFileModifiedTime(path, last_modified_file_2);
		//DEBUG_PRINT("ultima modificacao user: %s\n", last_modified_file_2);
		if(!fileExists(path) || older_file(last_modified, last_modified_file_2) == 1) {
			get_file(file_name, NULL);
		} else {
			strcpy(buffer, S_OK);
			status = write(sockid, buffer, BUFFER_SIZE);
      if (status < 0) {
        DEBUG_PRINT("ERROR writing to socket\n");
      }
		}
	}

  DEBUG_PRINT("Encerrando sincronização local.\n");
}

void syncronize_server(int sockid) {
  FileInfo localFiles[MAXFILES];
  char path[MAXNAME * 3 + 1];
  char buffer[BUFFER_SIZE];
  int number_files_client = 0;

  int status;

  DEBUG_PRINT("Iniciando sincronização do servidor.\n");

  number_files_client = get_dir_file_info(user.folder, localFiles);
	sprintf(buffer, "%d", number_files_client);
	status = write(sockid, buffer, BUFFER_SIZE);
  if (status < 0) {
    DEBUG_PRINT("ERROR writing to socket\n");
  }

	for(int i = 0; i < number_files_client; i++) {
		strcpy(buffer, localFiles[i].name);
		status = write(sockid, buffer, BUFFER_SIZE);
    if (status < 0) {
      DEBUG_PRINT("ERROR writing to socket\n");
    }

		strcpy(buffer, localFiles[i].last_modified);
		status = write(sockid, buffer, BUFFER_SIZE);
    if (status < 0) {
      DEBUG_PRINT("ERROR writing to socket\n");
    }

		status = read(sockid, buffer, BUFFER_SIZE);
    if (status < 0) {
      DEBUG_PRINT("ERROR reading from socket\n");
    }
		if(strcmp(buffer, S_GET) == 0) {
			sprintf(path, "%s/%s", user.folder, localFiles[i].name);
			send_file(path, FALSE);
		}
	}

  DEBUG_PRINT("Encerrando sincronização do servidor.\n");
}
