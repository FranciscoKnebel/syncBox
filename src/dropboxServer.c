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

void clearBuffer(){
  int i;
  for(i = 0; i < BUFFER_SIZE; i++)
  buffer[i]=NULL;
}

void error(char *msg){ // handle error
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]){ // ./dropboxServer endereço porta

  int status;
  char* address;
  int port = 5100;
 

  struct sockaddr_in server;
  server.sin_family = AF_INET; // address format is host and port number
  server.sin_port = htons(port); // host to network short
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  
  if(argc > 2){ // endereço e porta
    address = malloc(sizeof(argv[1]));
    strcpy(address, argv[1]);
    port = atoi(argv[2]);
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(address); 
  } else if(argc > 1){ //apenas endereço
      address = malloc(sizeof(argv[1]));
      strcpy(address, argv[1]);
      server.sin_addr.s_addr = inet_addr(address);
  } else{
      address = malloc(sizeof("127.0.0.1"));
      strcpy(address, "127.0.0.1");
  }
 
  sprintf(serverInfo.folder, "%s/syncBox_users", getUserHome());
  serverInfo.port = port;
  strcpy(serverInfo.ip, address);

  int sockid = socket(PF_INET, SOCK_STREAM, 0);

  if(bind(sockid, (struct sockaddr *) &server, sizeof(server))== -1) { // 0 = ok; -1 = erro, ele já faz o handle do erro
    printf("\nFalha no Bind\n");
    return 0;
  }


  int listen_status = listen(sockid, 1);
  if(listen_status == -1){
    printf("\nListening Error\n");
  } else{
      if(!fileExists(serverInfo.folder)) {
        if(mkdir(serverInfo.folder, 0777) != 0) {
	  printf("Error creating server folder '%s'.\n", serverInfo.folder);
          return;
	}
      }
      printf("Pasta do servidor: %s\n", serverInfo.folder);
      printf("Endereço do servidor: %s\n", serverInfo.ip);
      printf("Porta do servidor: %d\n", port);
      printf("Servidor no ar! Esperando conexões...\n");
    }


  while(1){

    struct sockaddr_in client;

    int cliLen = sizeof(struct sockaddr_in);

    int new_client_socket=accept(sockid,(struct sockaddr *)&client,&cliLen);
    
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

    char server_new_client_folder[2*MAXNAME];
    sprintf(server_new_client_folder, "%s/%s", serverInfo.folder, client_id);
    if(!fileExists(server_new_client_folder)) {
    	if(mkdir(server_new_client_folder, 0777) != 0) {
	  printf("Error creating user folder in server '%s'.\n", server_new_client_folder);
    	  return;
	}    
    }

    printf("\nConexão de %s através do IP %s \n", client_id, client_ip);
    clearBuffer(); // clears the variable buffer
   }

  close(sockid);

  return 0;
}
