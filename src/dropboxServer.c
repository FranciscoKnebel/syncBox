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

#define max_request 4

char buffer[BUFFER_SIZE];

void clearBuffer() {
  int i;
  for(i = 0; i < BUFFER_SIZE; i++)
    buffer[i] = 0;
}

void error(char *msg){ // handle error
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

  int port = DEFAULT_PORT;
  struct sockaddr_in server;
  char* address = malloc(strlen(DEFAULT_ADDRESS));

  parseArguments(argc, argv, address, &port, &server);
  server.sin_family = AF_INET; // address format is host and port number
  server.sin_port = htons(port); // host to network short
  server.sin_addr.s_addr = inet_addr(address);

  sprintf(serverInfo.folder, "%s/syncBox_users", getUserHome());
  serverInfo.port = port;
  strcpy(serverInfo.ip, address);

  int sockid = socket(PF_INET, SOCK_STREAM, 0);

  if(bind(sockid, (struct sockaddr *) &server, sizeof(server)) == -1) { // 0 = ok; -1 = erro, ele já faz o handle do erro
    printf("\nFalha no Bind\n");
    return ERROR_ON_BIND;
  }


  int listen_status = listen(sockid, 1);
  if(listen_status == -1){
    printf("\nListening Error\n");
  } else {
    if(!fileExists(serverInfo.folder)) {
      if(mkdir(serverInfo.folder, 0777) != 0) {
         printf("Error creating server folder '%s'.\n", serverInfo.folder);
         return ERROR_CREATING_SERVER_FOLDER;
      }
    }

    printf("Pasta do servidor: %s\n", serverInfo.folder);
    printf("Endereço do servidor: %s\n", serverInfo.ip);
    printf("Porta do servidor: %d\n", port);
    printf("Servidor no ar! Esperando conexões...\n");
  }


  while(1) {
    struct sockaddr_in client;

    unsigned int cliLen = sizeof(struct sockaddr_in);

    int new_client_socket=accept(sockid, (struct sockaddr *) &client, &cliLen);

    char client_id[MAXNAME];
    char *client_ip;

    client_ip = inet_ntoa(client.sin_addr); // inet_ntoa converte o IP de numeros e pontos para uma struct in_addr

    status = recv(new_client_socket, buffer, BUFFER_SIZE, 0);

    if(status != -1){
      strcpy(buffer, "conectado");
      status=send(new_client_socket, buffer, BUFFER_SIZE, 0);
    }
    strncpy(client_id, buffer, MAXNAME);
    client_id[MAXNAME] = '\0';

    char server_new_client_folder[2*MAXNAME +1];
    sprintf(server_new_client_folder, "%s/%s", serverInfo.folder, client_id);
    if(!fileExists(server_new_client_folder)) {
    	if(mkdir(server_new_client_folder, 0777) != 0) {
        printf("Error creating user folder in server '%s'.\n", server_new_client_folder);
        return ERROR_CREATING_USER_FOLDER;
      }
    }

    printf("\nConexão de %s através do IP %s \n", client_id, client_ip);
    clearBuffer(); // clears the variable buffer
  }

  close(sockid); // nunca chega aqui, while(1). Loop precisa quebrar pra fechar o socket.

  return 0;
}
