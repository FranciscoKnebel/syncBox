#include "dropboxServer.h"


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
  } else if(strcmp(buffer, S_DELETE) == 0){
    DEBUG_PRINT("\ndelete\n");
    do_delete(socket, client);
  }
}

void upload(int socket, Client* client){
  int status = 0;
  char buffer[BUFFER_SIZE]; // 1 KB buffer
  strcpy(buffer, S_NAME);
  status = write(socket, buffer, BUFFER_SIZE);

  char filename[MAXNAME];

  status = read(socket, buffer, BUFFER_SIZE);

  getLastStringElement(filename, buffer, "/");

  char file[MAXNAME];
  strcpy(file, filename);
  //printf("%s\n",file);

  sprintf(filename, "%s/%s/%s", serverInfo.folder, client->userid, file);

  pthread_mutex_lock(&mutex); // seção crítica 
  sockid_upload = socket; 
  receive_file(filename); 
  pthread_mutex_unlock (&mutex); // fim da seção crítica 
}

void download(int socket, Client* client){
  int status = 0;
  char buffer[BUFFER_SIZE]; // 1 KB buffer
  strcpy(buffer, S_NAME);
  status = write(socket, buffer, BUFFER_SIZE);

  char filename[MAXNAME];

  status = read(socket, buffer, BUFFER_SIZE);

  strcpy(filename, buffer);
  char file[MAXNAME];
  strcpy(file, filename);
  //printf("%s\n",file);

  sprintf(filename, "%s/%s/%s", serverInfo.folder, client->userid, file);

  pthread_mutex_lock(&mutex); // seção crítica 
  sockid_download = socket; 
  send_file(filename); 
  pthread_mutex_unlock (&mutex); // fim da SC

}

void list_server(int socket, Client* client){
  int status = 0;
  char buffer[BUFFER_SIZE]; // 1 KB buffer

  sprintf(buffer, "%d", client->n_files);
  //printf("number files: %d\n", atoi(buffer));
  status = write(socket, buffer, BUFFER_SIZE);
  for(int i = 0; i < client->n_files; i++) {
	sprintf(buffer, "%s Modification time: %s", client->file_info[i].name, client->file_info[i].last_modified); 
    	status = write(socket, buffer, BUFFER_SIZE); 
  }
}

void do_delete(int socket, Client* client){ 
    int status = 0; 
    char buffer[BUFFER_SIZE]; // 1 KB buffer
    strcpy(buffer, S_NAME); 
    status = write(socket, buffer, BUFFER_SIZE); 
 
    char filename[MAXNAME]; 
 
    status = read(socket, buffer, BUFFER_SIZE); 
 
    getLastStringElement(filename, buffer, "/"); 
 
    char file[MAXNAME]; 
    strcpy(file, filename); 
    //printf("%s\n",file); 
 
    sprintf(filename, "%s/%s/%s", serverInfo.folder, client->userid, file); 
    //printf("deletando arquivo %s\n", filename); 
         
    if(fileExists(filename)){
    	if(remove(filename)!=0) { 
        	printf("Erro ao deletar o arquivo %s\n", filename); 
    	} else{ 
    		printf("Arquivo %s excluido!\n", filename); 
	}

        strcpy(buffer, S_DELETED); 
    	status = write(socket, buffer, BUFFER_SIZE); 
    }  
}

