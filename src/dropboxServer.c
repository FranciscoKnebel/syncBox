#include "dropboxServer.h"

ServerInfo serverInfo;
Client clients[MAX_CLIENTS];
char buffer[BUFFER_SIZE];


pthread_mutex_t mutex; 
pthread_mutex_t mutex_sync; 

int sockid_upload = 0; 
int sockid_download = 0; 
int sockid_sync = 0; 
 
Client* client_sync; 

void sync_server() {
  // sincroniza o cliente com servidor e servidor com cliente
  int status = 0; 
  char buffer[BUFFER_SIZE]; // 1 KB buffer 
  int number_files_client = 0;
  char file_name[MAXNAME];
  char last_modified[MAXNAME];
  char path[MAXNAME * 2];

  status = read(sockid_sync, buffer, BUFFER_SIZE); 
  if(strcmp(buffer, S_SYNC) == 0){
	DEBUG_PRINT("sincronizar!\n");
  }

  sprintf(buffer, "%d", client_sync->n_files); 
  status = write(sockid_sync, buffer, BUFFER_SIZE); 

  for(int i = 0; i < client_sync->n_files; i++){
	    strcpy(buffer, client_sync->file_info[i].name);
	    status = write(sockid_sync, buffer, BUFFER_SIZE); 
	    strcpy(buffer, client_sync->file_info[i].last_modified);
	    status = write(sockid_sync, buffer, BUFFER_SIZE); 
	    status = read(sockid_sync, buffer, BUFFER_SIZE); 
	    if(strcmp(buffer, S_DOWNLOAD) == 0){
	    	download(sockid_sync, client_sync);
	    }
  }
  // sincroniza agora o servidor com o cliente
  status = read(sockid_sync, buffer, BUFFER_SIZE);
  number_files_client = atoi(buffer);
  printf("Number files client: %d\n", number_files_client);
  char last_modified_file_2[MAXNAME];
  for(int i = 0; i < number_files_client; i++){
	status = read(sockid_sync, buffer, BUFFER_SIZE);
        strcpy(file_name, buffer);
	//printf("%s\n", file_name);
	status = read(sockid_sync, buffer, BUFFER_SIZE);
        strcpy(last_modified, buffer);
        //printf("ultima modificaçao server: %s\n", last_modified);
        sprintf(path, "%s/%s/%s", serverInfo.folder, client_sync->userid, file_name);
	getFileModifiedTime(&path, &last_modified_file_2);
        //printf("ultima modificacao user: %s\n", last_modified_file_2);
        if(!fileExists(path) || older_file(&last_modified, &last_modified_file_2) == 1){
		strcpy(buffer, S_GET);
		status = write(sockid_sync, buffer, BUFFER_SIZE); 
                status = read(sockid_sync, buffer, BUFFER_SIZE);
		if(strcmp(buffer, S_UPLOAD) == 0){
			upload(sockid_sync, client_sync);
		}	
	} else{
		strcpy(buffer, "OK");
		status = write(sockid_sync, buffer, BUFFER_SIZE);	
	}
  }




  DEBUG_PRINT("sincronizacao finalizada!\n");
   
}


void receive_file(char *file){
  int bytes_written = 0;
  int status = 0;
  int file_size = 0;

  DEBUG_PRINT("recebendo %s\n", file);

  FILE* pFile;
  char buffer[BUFFER_SIZE]; // 1 KB buffer

  pFile = fopen(file, "wb");
  if(pFile) {

    //requisita o arquivo file do cliente
    // recebe buffer do servidor
    status = read(sockid_upload, buffer, BUFFER_SIZE);
    file_size = atoi(buffer);

    status = 0;
    int bytes_to_read = file_size;
    while(file_size > bytes_written) {
      status = read(sockid_upload, buffer, BUFFER_SIZE); // le no buffer
      if(bytes_to_read > BUFFER_SIZE){ // se o tamanho do arquivo for maior, lê buffer completo
        fwrite(buffer, sizeof(char), BUFFER_SIZE, pFile);
        bytes_written += sizeof(char) * BUFFER_SIZE;
        bytes_to_read -= bytes_to_read;
      } else{ // senão lê só o file_size
         fwrite(buffer, sizeof(char), bytes_to_read, pFile);
         bytes_written += sizeof(char) * bytes_to_read;
      }
      DEBUG_PRINT("leu\n");
    }
    fclose(pFile);

    DEBUG_PRINT("Arquivo %s salvo.\n", file);
  } else {
    DEBUG_PRINT("Erro abrindo arquivo %s.\n", file);
  }
}

void send_file(char *file) {
  int file_size = 0;
  int bytes_read = 0;
  int status = 0;

  FILE* pFile;
  char buffer[BUFFER_SIZE]; // 1 KB buffer

  pFile = fopen(file, "rb");
  if(pFile) {

    file_size = getFilesize(pFile);
    DEBUG_PRINT("file size: %d\n", file_size);
    sprintf(buffer, "%d", file_size); // envia tamanho do arquivo para o cliente
    status = write(sockid_download, buffer, BUFFER_SIZE);

    if(file_size == 0) {
      fclose(pFile);
      return;
    }

    while(!feof(pFile)) {
        fread(buffer, sizeof(char), BUFFER_SIZE, pFile);
        bytes_read += sizeof(char) * BUFFER_SIZE;

        // enviar buffer para salvar no cliente
        status = write(sockid_download, buffer, BUFFER_SIZE);
    }

    fclose(pFile);
    DEBUG_PRINT("Arquivo %s enviado.\n", file);
  } else {
    DEBUG_PRINT("Erro abrindo arquivo %s.\n", file);
  }
}

void parseArguments(int argc, char *argv[], char* address, int* port, struct sockaddr_in* server) {
  if(argc > 2) { // endereço e porta
    strcpy(address, argv[1]);
    *port = atoi(argv[2]);
  } else if(argc == 2) { // apenas endereço
    strcpy(address, argv[1]);
    *port = DEFAULT_PORT;
  } else {
    strcpy(address, DEFAULT_ADDRESS);
    *port = DEFAULT_PORT;
  }
}

void* continueClientProcess(Connection* connection) {
  int socket = connection->socket_id;
  char* client_ip = connection->ip;
  char client_id[MAXNAME];
  int status;
  int device = 0;
  Client* client;

  bzero(buffer, BUFFER_SIZE);

  // read
  status = read(socket, buffer, BUFFER_SIZE);
  if (status < 0) {
    DEBUG_PRINT("ERROR reading from socket\n");
    exit(1);
  }
  // do stuff...
  strncpy(client_id, buffer, MAXNAME);
  client_id[MAXNAME] = '\0';

  strcpy(buffer, S_CONNECTED);

  int position = 0;
  position = searchClient(client, client_id);

  if(position == CLIENT_NOTFOUND) {
    position = newClient(client_id, socket);
    client = &clients[position];
  } else {
    client = &clients[position];
    device = addDevice(client, socket);

    if(device == -1) {
      strcpy(buffer, S_EXCESS_DEVICES);
    }
  }

  printf("socket: %d - position: %d - device: %d\n", socket, position, device);

  //DEBUG_PRINT("\n%d number_files, %s arquivo 1, %s arquivo 2\n", client->n_files, client->file_info[0].name, client->file_info[1].name);

  if(device != -1) {
    char client_folder[2*MAXNAME +1];

    sprintf(client_folder, "%s/%s", serverInfo.folder, client_id);
    if(!fileExists(client_folder)) {
      if(mkdir(client_folder, 0777) != 0) {
        printf("Error creating user folder in server '%s'.\n", client_folder);
        return ERROR_CREATING_USER_FOLDER;
      }
    }

    printf("Conexão iniciada do usuário '%s%s%s' através do IP '%s%s%s'.\n",
    COLOR_GREEN, client_id, COLOR_RESET, COLOR_GREEN, client_ip, COLOR_RESET);

    status = write(socket, buffer, BUFFER_SIZE);
    
    pthread_mutex_lock(&mutex_sync); // seção crítica 
    sockid_sync = socket; 
    client_sync = client;
    sync_server();
    pthread_mutex_unlock (&mutex_sync); // fim da seção crítica 


    if (status < 0) {
      DEBUG_PRINT("ERROR writing to socket\n");
      exit(1);
    }

    int disconnected = 0;
    do {
      bzero(buffer, BUFFER_SIZE);

      // read
      status = read(socket, buffer, BUFFER_SIZE);
      if (status < 0) {
        DEBUG_PRINT("ERROR reading from socket");
        exit(1);
      }

      if(strcmp(buffer, S_REQ_DC) == 0) {
        strcpy(buffer, S_RPL_DC);
        // write
        status = write(socket, buffer, BUFFER_SIZE);
        if (status < 0) {
          printf("ERROR writing to socket\n");
          exit(1);
        }

        disconnected = 1;
      } else {
        select_commands(socket, buffer, client);
      }
    } while(disconnected != 1);

    printf("%s desconectou no dispositivo %d, socket %d!\n", client_id, removeDevice(client, device), socket);
  } else {
    // write
    status = write(socket, buffer, BUFFER_SIZE);
    if (status < 0) {
      DEBUG_PRINT("ERROR writing to socket\n");
      exit(1);
    }
  }
}

int main(int argc, char *argv[]){ // ./dropboxServer endereço porta
  int status;

  int port = DEFAULT_PORT;
  struct sockaddr_in server, client;

  clearClients();
  char* address = malloc(strlen(DEFAULT_ADDRESS));

  /* Initialize socket structure */
  bzero((char *) &server, sizeof(server));

  parseArguments(argc, argv, address, &port, &server);
  server.sin_family = AF_INET; // address format is host and port number
  server.sin_port = htons(port); // host to network short
  server.sin_addr.s_addr = inet_addr(address);

  sprintf(serverInfo.folder, "%s/syncBox_users", getUserHome());
  strcpy(serverInfo.ip, address);
  serverInfo.port = port;

  // Criação e nomeação de socket
  int sockid = socket(PF_INET, SOCK_STREAM, 0);
  if(bind(sockid, (struct sockaddr *) &server, sizeof(server)) == -1) { // 0 = ok; -1 = erro, ele já faz o handle do erro
    perror("Falha na nomeação do socket");
    return ERROR_ON_BIND;
  }

  // Criação da pasta de armazenamento
  if(!fileExists(serverInfo.folder)) {
    if(mkdir(serverInfo.folder, 0777) != 0) {
      printf("Error creating server folder '%s'.\n", serverInfo.folder);
      return ERROR_CREATING_SERVER_FOLDER;
    }
  }

  printf("Pasta do servidor: %s%s%s\n", COLOR_GREEN, serverInfo.folder, COLOR_RESET);
  printf("Endereço do servidor: %s%s%s\n", COLOR_GREEN, serverInfo.ip, COLOR_RESET);
  printf("Porta do servidor: %s%d%s\n", COLOR_GREEN, serverInfo.port, COLOR_RESET);

  int listen_status = listen(sockid, MAX_CLIENTS); // segundo argumento é a quantidade de clientes que o socket vai fazer o listen
  if(listen_status == -1) {
    printf("\nListening Error\n");
  } else {
    printf("Servidor no ar! Esperando conexões...\n");
  }

  pthread_t thread_id;

  while(1) {
    unsigned int cliLen = sizeof(struct sockaddr_in);

    int new_client_socket = accept(sockid, (struct sockaddr *) &client, &cliLen);

    if(new_client_socket < 0) {
	     printf("Error on accept\n");
    }

    char *client_ip = inet_ntoa(client.sin_addr); // inet_ntoa converte o IP de numeros e pontos para uma struct in_addr

    Connection *connection = malloc(sizeof(*connection));
    connection->socket_id = new_client_socket;
    connection->ip = client_ip;

    if(pthread_create(&thread_id, NULL, continueClientProcess, connection) < 0){
      printf("Error on create thread\n");
    }
  }

  return 0;
}
