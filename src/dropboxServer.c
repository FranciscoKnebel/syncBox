#include "dropboxServer.h"

ServerInfo serverInfo;
Client clients[MAX_CLIENTS];

void sync_server() {
  return;
}

void receive_file(char *file) {
  return;
}

void send_file(char *file) {
  return;
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

int main(int argc, char *argv[]){ // ./dropboxServer endereço porta
  int status;
  int pid;

  int port = DEFAULT_PORT;
  struct sockaddr_in server, client;

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

   strcpy(buffer, "conectado");

   Client* client = searchClient(&client_id);

   int retorno_addDevice = 0;
   if(client == NULL){
   	printf("client pos: %d\n", newClient(client_id));
	client = searchClient(&client_id);
   } else {
	retorno_addDevice = addDevice(client);
	printf("device: %d\n", retorno_addDevice);
	if(retorno_addDevice == -1){
		strcpy(buffer, "excess devices");
	}
   }
   if(retorno_addDevice != -1){

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

	     if(strcmp(buffer, "disconnect") == 0){
	       strcpy(buffer, "disconnected");
	       // write
	       status = write(socket, buffer, BUFFER_SIZE);
	       if (status < 0) {
		printf("ERROR writing to socket\n");
		exit(1);
	       }
	       disconnected = 1;
	     } else {

	     	// write
	     	status = write(socket, buffer, BUFFER_SIZE);
	     	if (status < 0) {
		  printf("ERROR writing to socket\n");
		  exit(1);
		}

	    }
	  } while(disconnected != 1);
	  
	  printf("%s desconectou no dispositivo %d!\n", client_id, removeDevice(client));
  } else{
    	   // write
	   status = write(socket, buffer, BUFFER_SIZE);
	   if (status < 0) {
	      printf("ERROR writing to socket\n");
	      exit(1);
	   }
    }

}

Client* searchClient(char* userId){
	int i;
	for(i = 0; i < MAX_CLIENTS; i++){
		if(strcmp(userId, clients[i].userid) == 0){
			return &clients[i];		
		}
	}
	return NULL;
}

int newClient(char* userid){
	int i;
	for(i = 0; i < MAX_CLIENTS; i++){
		if(!clients[i].logged_in){
                        clients[i].devices[0] = 1;
			strcpy(clients[i].userid, userid);
			// TODO search files... and number of files
			clients[i].logged_in = 1;
			return i;
		}
	}
	return -1; // cheio de usuários
}

int addDevice(Client* client){
	if(client->devices[0] == 0){
     		client->devices[0] = 1;
		return 0;
        }
        if(client->devices[1] == 0){
     		client->devices[1] = 1;
		return 1;
        }
	return -1;
}

int removeDevice(Client* client){ // TODO rever... se passar parametro do dispositivo ou o que fazer.
	if(client){
		if(client->devices[0] == 1){
     			client->devices[0] = 0;
			return 0;
        	}
        	if(client->devices[1] == 1){
     			client->devices[1] = 0;
			return 1;
        	}
	}
	return -1;
}

void check_login_status(Client* client){
	if(client->devices[0] == 0 && client->devices[1] == 0){
		client->logged_in == 0;
		printf("Cliente %s logged out!", client->userid);
	}
}


