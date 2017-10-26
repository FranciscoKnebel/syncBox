#include "dropboxServer.h"

ServerInfo serverInfo;
Client clients[MAX_CLIENTS];

void sync_server() {
  return;
}

void receive_file(char *file){
  int sockid = 4;
  int bytes_written = 0;
  int status = 0;
  int file_size = 0;

  printf("recebendo %s\n", file);

  FILE* pFile;
  char buffer[BUFFER_SIZE]; // 1 KB buffer

  pFile = fopen(file, "wb");
  if(pFile) {

    //requisita o arquivo file do cliente
    // recebe buffer do servidor
    status = read(sockid, buffer, BUFFER_SIZE);
    file_size = atoi(buffer);

    status = 0;
    int bytes_to_read = file_size;
    while(file_size > bytes_written) {
      status = read(sockid, buffer, BUFFER_SIZE); // le no buffer
      if(bytes_to_read > BUFFER_SIZE){ // se o tamanho do arquivo for maior, lê buffer completo
        fwrite(buffer, sizeof(char), BUFFER_SIZE, pFile);
        bytes_written += sizeof(char) * BUFFER_SIZE;
        bytes_to_read -= bytes_to_read;
      } else{ // senão lê só o file_size
         fwrite(buffer, sizeof(char), bytes_to_read, pFile);
         bytes_written += sizeof(char) * bytes_to_read;
      }
      printf("leu\n");
    }
    fclose(pFile);

    printf("Arquivo %s salvo.\n", file);
  } else {
    printf("Erro abrindo arquivo %s.\n", file);
  }
}

void send_file(char *file) {
  int sockid = 4;
  int file_size = 0;
  int bytes_read = 0;
  int status = 0;

  FILE* pFile;
  char buffer[BUFFER_SIZE]; // 1 KB buffer

  pFile = fopen(file, "rb");
  if(pFile) {

    file_size = getFilesize(pFile);
    printf("file size: %d\n", file_size);
    sprintf(buffer, "%d", file_size); // envia tamanho do arquivo para o cliente
    status = write(sockid, buffer, BUFFER_SIZE);

    if(file_size == 0) {
      fclose(pFile);
      return;
    }

    while(!feof(pFile)) {
        fread(buffer, sizeof(char), BUFFER_SIZE, pFile);
        bytes_read += sizeof(char) * BUFFER_SIZE;

        // enviar buffer para salvar no cliente
        status = write(sockid, buffer, BUFFER_SIZE);
    }

    fclose(pFile);
    printf("Arquivo %s enviado.\n", file);
  } else {
    printf("Erro abrindo arquivo %s.\n", file);
  }
}


char buffer[BUFFER_SIZE];

void clearBuffer() {
  int i;
  for(i = 0; i < BUFFER_SIZE; i++){
    buffer[i] = 0;
  }
}

void error(char *msg) { // handle error
  perror(msg);
  exit(1);
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

void clearClients(){
	for(int i = 0; i < MAX_CLIENTS; i++){
		clients[i].logged_in = 0;
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

  printf("Pasta do servidor: %s%s%s\n", ANSI_COLOR_GREEN, serverInfo.folder, ANSI_COLOR_RESET);
  printf("Endereço do servidor: %s%s%s\n", ANSI_COLOR_GREEN, serverInfo.ip, ANSI_COLOR_RESET);
  printf("Porta do servidor: %s%d%s\n", ANSI_COLOR_GREEN, serverInfo.port, ANSI_COLOR_RESET);



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


    if(pthread_create( &thread_id , NULL ,  continueClientProcess, connection) < 0){
            printf("Error on create thread\n");
            exit(1);
    }
  }

  return 0;
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
      printf("ERROR reading from socket\n");
      exit(1);
   }
   // do stuff...
   strncpy(client_id, buffer, MAXNAME);
   client_id[MAXNAME] = '\0';

   strcpy(buffer, S_CONNECTED);

   int clientPos = 0;

   clientPos = searchClient(client, client_id);

   if(clientPos == -1){
	clientPos = newClient(client_id, socket);
	client = &clients[clientPos];
   } else {
	client = &clients[clientPos];
	device = addDevice(client, socket);
	if(device == -1){
		strcpy(buffer, S_EXCESS_DEVICES);
	}
   }

   printf("socket: %d\n", socket);
   printf("client pos: %d\n", clientPos);
   printf("device: %d\n", device);

   // debug
   //printf("\n%d number_files, %s arquivo 1, %s arquivo 2\n", client->n_files, client->file_info[0].name, client->file_info[1].name);

   if(device != -1){

	   char server_new_client_folder[2*MAXNAME +1];

	   sprintf(server_new_client_folder, "%s/%s", serverInfo.folder, client_id);
	   if(!fileExists(server_new_client_folder)) {
		if(mkdir(server_new_client_folder, 0777) != 0) {
			printf("Error creating user folder in server '%s'.\n", server_new_client_folder);
			return ERROR_CREATING_USER_FOLDER;
		}
	   }

	   printf("Conexão iniciada do usuário '%s%s%s' através do IP '%s%s%s'.\n", ANSI_COLOR_GREEN, client_id, ANSI_COLOR_RESET,
	   ANSI_COLOR_GREEN, client_ip, ANSI_COLOR_RESET);


	   // write
	   status = write(socket, buffer, BUFFER_SIZE);
	   if (status < 0) {
	      printf("ERROR writing to socket\n");
	      exit(1);
	   }

	   int disconnected = 0;

	   do{
	     bzero(buffer, BUFFER_SIZE);

	     // read
	     status = read(socket, buffer, BUFFER_SIZE);
	     if (status < 0) {
		printf("ERROR reading from socket");
		exit(1);
	     }

	     if(strcmp(buffer, S_REQ_DC) == 0){
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
  } else{
    	   // write
	   status = write(socket, buffer, BUFFER_SIZE);
	   if (status < 0) {
	      printf("ERROR writing to socket\n");
	      exit(1);
	   }
    }


}

int searchClient(Client* client, char* userId){
	int i;
	for(i = 0; i < MAX_CLIENTS; i++){
		if(strcmp(userId, clients[i].userid) == 0){
			if(clients[i].logged_in == 1){
				client = &clients[i];
				return i;
			}
		}
	}
	return -1;
}

int newClient(char* userid, int socket){
	int i;
	for(i = 0; i < MAX_CLIENTS; i++){
		if(!clients[i].logged_in){
                        clients[i].devices[0] = socket;
			clients[i].devices[1] = -1;
			strcpy(clients[i].userid, userid);
			char server_new_client_folder[2*MAXNAME +1];
	   		sprintf(server_new_client_folder, "%s/%s", serverInfo.folder, userid);
			clients[i].n_files = get_dir_content_file_info(&server_new_client_folder, clients[i].file_info);
			clients[i].logged_in = 1;
			return i;
		}
	}
	return -1; // cheio de usuários
}

int addDevice(Client* client, int socket){
	if(client->devices[0] == -1){
     		client->devices[0] = socket;
		return 0;
        }
        if(client->devices[1] == -1){
     		client->devices[1] = socket;
		return 1;
        }
	return -1;
}

int removeDevice(Client* client, int device){
	if(client){
		client->devices[device] = -1;
		return device;
	}
	return -1;
}

void check_login_status(Client* client){
	if(client->devices[0] == 0 && client->devices[1] == 0){
		client->logged_in = 0;
		printf("Cliente %s logged out!", client->userid);
	}
}

void select_commands(int socket, char buffer[], Client* client){
  if(strcmp(buffer, S_UPLOAD) == 0){
    printf("\nupload\n");
    do_upload(socket, buffer, client);
  } else if(strcmp(buffer, S_DOWNLOAD) == 0){
    printf("\ndownload\n");
    do_download(socket, buffer, client);
  } else if(strcmp(buffer, S_LS) == 0){
    printf("\nlist_server\n");
    do_list_server(socket, buffer, client);
  }
}

void do_upload(int socket, char buffer[], Client* client){
  int status = 0;
  strcpy(buffer, S_NAME);
  status = write(socket, buffer, BUFFER_SIZE);

  char filename[MAXNAME];

  status = read(socket, buffer, BUFFER_SIZE);

  getLastStringElement(filename, buffer, "/");

  char file[MAXNAME];
  strcpy(file, filename);
  printf("%s\n",file);

  sprintf(filename, "%s/%s/%s", serverInfo.folder, client->userid, file);

  receive_file(filename);

}

void do_download(int socket, char buffer[], Client* client){
  int status = 0;
  strcpy(buffer, S_NAME);
  status = write(socket, buffer, BUFFER_SIZE);

  char filename[MAXNAME];

  status = read(socket, buffer, BUFFER_SIZE);

  strcpy(filename, buffer);
  char file[MAXNAME];
  strcpy(file, filename);
  printf("%s\n",file);

  sprintf(filename, "%s/%s/%s", serverInfo.folder, client->userid, file);

  send_file(filename);

}

void do_list_server(int socket, char buffer[], Client* client){
	int sockid = 4;
	int status = 0;
        sprintf(buffer, "%d", client->n_files);
	printf("number files: %d\n", atoi(buffer));
	status = write(sockid, buffer, BUFFER_SIZE);
	for(int i = 0; i < client->n_files; i++){
		strcpy(buffer, client->file_info[i].name);
		status = write(sockid, buffer, BUFFER_SIZE);
	}
}
