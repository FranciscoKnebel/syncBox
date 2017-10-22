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
  for(i = 0; i < BUFFER_SIZE; i++)
    buffer[i] = 0;
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

  int port = DEFAULT_PORT;
  struct sockaddr_in server;
  char* address = malloc(strlen(DEFAULT_ADDRESS));

  parseArguments(argc, argv, address, &port, &server);
  server.sin_family = AF_INET; // address format is host and port number
  server.sin_port = htons(port); // host to network short
  server.sin else {_addr.s_addr = inet_addr(address);

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

  // Ouvindo o socket
  int listen_status = listen(sockid, SOCKET_BACKLOG);
  if(listen_status == -1) {
    perror("Listening Error");
  } else {
    printf("Pasta do servidor: %s%s%s\n", ANSI_COLOR_GREEN, serverInfo.folder, ANSI_COLOR_RESET);
    printf("Endereço do servidor: %s%s%s\n", ANSI_COLOR_GREEN, serverInfo.ip, ANSI_COLOR_RESET);
    printf("Porta do servidor: %s%d%s\n", ANSI_COLOR_GREEN, serverInfo.port, ANSI_COLOR_RESET);
    printf("Servidor no ar! Esperando conexões...\n");
  }

  while(1) {
    struct sockaddr_in client;

    unsigned int cliLen = sizeof(struct sockaddr_in);

    int new_client_socket = accept(sockid, (struct sockaddr *) &client, &cliLen);

    char client_id[MAXNAME];
    char *client_ip;

    client_ip = inet_ntoa(client.sin_addr); // inet_ntoa converte o IP de numeros e pontos para uma struct in_addr

    status = recv(new_client_socket, buffer, MAXNAME, 0);
    if(status == -1) {
      perror("Erro ao receber conexão");
    } else {
      // Salva id do cliente conectado
      strncpy(client_id, buffer, MAXNAME);
      client_id[MAXNAME] = '\0';

      // Enviar id de volta, como confirmação de conexão para o cliente
      strcpy(buffer, client_id);
      status = send(new_client_socket, buffer, MAXNAME, 0);
    }

    char server_new_client_folder[2*MAXNAME +1];
    sprintf(server_new_client_folder, "%s/%s", serverInfo.folder, client_id);
    if(!fileExists(server_new_client_folder)) {
    	if(mkdir(server_new_client_folder, 0777) != 0) {
        printf("Erro criando pasta do usuário '%s'.\n", server_new_client_folder);
        return ERROR_CREATING_USER_FOLDER;
      }
    }

    printf("Conexão iniciada do usuário '%s%s%s' através do IP '%s%s%s'.\n", ANSI_COLOR_GREEN, client_id, ANSI_COLOR_RESET,
    ANSI_COLOR_GREEN, client_ip, ANSI_COLOR_RESET);

    clearBuffer();
  }

  close(sockid); // nunca chega aqui, while(1). Loop precisa quebrar pra fechar o socket.

  return 0;
}
