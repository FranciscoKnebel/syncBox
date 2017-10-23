#include "dropboxServer.h"

ServerInfo serverInfo;

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



  int listen_status = listen(sockid, MAX_CLIENT_LISTENED); // segundo argumento é a quantidade de clientes que o socket vai fazer o listen

  if(listen_status == -1) {
    printf("\nListening Error\n");
  } else {
    printf("Servidor no ar! Esperando conexões...\n");
  }

  while(1) {

    unsigned int cliLen = sizeof(struct sockaddr_in);

    int new_client_socket = accept(sockid, (struct sockaddr *) &client, &cliLen);

    if(new_client_socket < 0) {
	     printf("Error on accept\n");
    }

    pid = fork(); // Create child process
    if (pid < 0) {
      printf("ERROR on fork\n");
      exit(1);
    }

    if (pid == 0) { // This is the client process
         close(sockid);
         char *client_ip = inet_ntoa(client.sin_addr); // inet_ntoa converte o IP de numeros e pontos para uma struct in_addr
         continueClientProcess(new_client_socket, client_ip);
         exit(0);
    } else {
         close(new_client_socket);
    }
  }

  return 0;
}

void continueClientProcess(int socket, char* client_ip) {
   char client_id[MAXNAME];
   int status;
   bzero(buffer, BUFFER_SIZE);

   // read
   status = read(socket, buffer, BUFFER_SIZE);
   if (status < 0) {
      perror("ERROR reading from socket");
      exit(1);
   }
   printf("%s\n",buffer); // debug
   // do stuff...
   strncpy(client_id, buffer, MAXNAME);
   client_id[MAXNAME] = '\0';

   strcpy(buffer, "conectado");

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

     	printf("recebido: %s\n",buffer); // debug

     	// write
     	status = write(socket, buffer, BUFFER_SIZE);
     	if (status < 0) {
          printf("ERROR writing to socket\n");
          exit(1);
        }

    }
  } while(disconnected != 1);
  printf("%s desconectou!\n", client_id);

}
