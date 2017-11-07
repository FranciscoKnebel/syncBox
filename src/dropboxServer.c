#include "dropboxServer.h"

ServerInfo serverInfo;
Client clients[MAX_CLIENTS];


char buffer[BUFFER_SIZE];

void sync_server() {
  return;
}

void receive_file(char *file,int socket) {

  char buffer[BUFFER_SIZE]; 

  int bytes_read = read(socket, buffer, BUFFER_SIZE);
  if(bytes_read < 0){
    printf("error reading from socket\n");
    return;
  }
  int file_size = atoi(buffer);

  FILE *f = fopen(file,"wb");
  if(f == NULL){
    printf("error creating file %s\n",file);
    return;
  }
  int total_bytes = 0;
  while(file_size > total_bytes){
    bytes_read = read(socket, buffer, BUFFER_SIZE);
    if(bytes_read < 0){
      printf("error reading from socket\n");
      return;
    }
    total_bytes += bytes_read;
    if(file_size < total_bytes)
      fwrite(buffer,sizeof(char),file_size % BUFFER_SIZE,f);
    else
      fwrite(buffer,sizeof(char),bytes_read,f);
  }
  fclose(f);

  printf("arquivo recebido com sucesso!\n");
  return;
}

void send_file(char *file,int sockid) {
  int file_size = 0;
  int bytes_read = 0;

  FILE* pFile;
  char buffer[BUFFER_SIZE]; // 1 KB buffer

  pFile = fopen(file, "rb");
  if(pFile) {
    file_size = getFilesize(pFile);

    if(file_size == 0) {
      fclose(pFile);
      return;
    }

    bzero(buffer, BUFFER_SIZE);
    sprintf(buffer,"%d",file_size);
    if(0 > write(sockid, buffer, BUFFER_SIZE)){
      printf("Error sending the size of the file!\n");
      return;
    }
    printf("envio de arquivo de %d bytes\n",file_size);

    int sent_bytes;
    while(!feof(pFile)) {

        
        bytes_read += fread(buffer, sizeof(char), BUFFER_SIZE, pFile);

        sent_bytes = write(sockid, buffer, BUFFER_SIZE);
          if (sent_bytes < 0) {
              printf("ERROR writing to socket\n");
              exit(1);
          }
          char *buffer_offset = buffer;
          while(sent_bytes != BUFFER_SIZE){
            buffer_offset = buffer + sent_bytes;
            sent_bytes += write(sockid, buffer_offset, BUFFER_SIZE);
          }
    }

    fclose(pFile);
    printf("arquivo %s enviado.\n", file);
  } else {
    printf("erro na abertura do arquivo %s.\n", file);
  }
}



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
	int i;
	for(i = 0; i < MAX_CLIENTS; i++){
		clients[i].logged_in = 0;	
	}
}

int main(int argc, char *argv[]){ // ./dropboxServer endereço porta
  
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
  int sockid = socket(AF_INET, SOCK_STREAM, 0);//era PF_INET, porém slides do sor confirmão AF_INET
  if(sockid == -1 ){
    perror("Falha na abertura do socket");
    return ERROR_ON_OPENING_SOCKET;
  }
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

  //leitura da árvore de usuários já existentes!
  startSem();
  char arquivo_de_persistencia[MAXNAME*3];
  sprintf(arquivo_de_persistencia,"%s/%s",serverInfo.folder,DEFAULT_USER_METADATA_FILE);

  printf("arquivo de persistência de dados:%s\n",arquivo_de_persistencia);

  int op_mod = get_clients_from_file(arquivo_de_persistencia);
  if(op_mod == 0)
    puts("houve recuperação de usuários");
  else
    puts("novo local, sem usuários");


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
    

    if(pthread_create( &thread_id , NULL ,  (void * (*)(void *))continueClientProcess, connection) < 0){
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

   strcpy(buffer, "conectado");

   int clientPos = 0;

   //parte envolvendo metadados, como professor pediu algo diferente de array, coloquei a árvore
   //se usuário é novo no dropbox ele tem 0 arquivos
   client = get_client(client_id);
   if(client == NULL){
    add_client(client_id);
    client = get_client(client_id);
   }
   device = addDevice(client);
   if(device == -1){
    strcpy(buffer, "excess devices");
    }

   /*
   
   clientPos = searchClient(client, &client_id);
   
   if(clientPos == -1){
	clientPos = newClient(client_id);
	client = &clients[clientPos];
   } else {
	client = &clients[clientPos];
	device = addDevice(client);
	if(device == -1){
		strcpy(buffer, "excess devices");
	}*/
   


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
          exit(1);
    			//return ERROR_CREATING_USER_FOLDER;
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

        //mapeando o que o cliente deseja
        switch(buffer[0]){
          case WANTS_SEND:
            status = read(socket, buffer, BUFFER_SIZE);
            if(status < 0){
              printf("ERROR reading from socket\n");
              exit(1);
            }
            printf("client is sending file %s\n",buffer);

            {
              char complete_path[MAXNAME];
              complete_path[0]='\0';
              strcat(complete_path,server_new_client_folder);
              strcat(complete_path,buffer);

              receive_file(complete_path,socket);
              if( 0 != get_file_from_client(client,complete_path,buffer)){
                add_file_to_client(client,complete_path);
              }
            }
            break;
          case WANTS_RECEIVE:
            status = read(socket, buffer, BUFFER_SIZE);
            if(status < 0){
              printf("ERROR reading from socket\n");
              exit(1);
            }
            printf("client wants file %s\n",buffer);
            
            {
              char complete_path[MAXNAME];
              complete_path[0]='\0';
              strcat(complete_path,server_new_client_folder);
              strcat(complete_path,buffer);

              if( 0 != get_file_from_client(client,complete_path,buffer)){
                printf("pedido de arquivo que não existe\n");
                bzero(buffer,BUFFER_SIZE);
                sprintf(buffer,"%s",FILE_DONT_EXIST);
                status = write(socket, buffer, BUFFER_SIZE);
                   if (status < 0) {
                    printf("ERROR writing to socket\n");
                    exit(1);
                   }

                break;
             }else
              send_file(complete_path,socket);
            }
            break;
          default:
            printf("unknown wish\n");
        }

	    }
	  } while(disconnected != 1);
	  
	  printf("%s desconectou no dispositivo %d!\n", client_id, removeDevice(client, device));
    char arquivo_de_persistencia[MAXNAME*3];
    sprintf(arquivo_de_persistencia,"%s/%s",serverInfo.folder,DEFAULT_USER_METADATA_FILE);
    save_clients(arquivo_de_persistencia);
  } else{
    	   // write
	   status = write(socket, buffer, BUFFER_SIZE);
	   if (status < 0) {
	      printf("ERROR writing to socket\n");
	      exit(1);
	   }
    }
    return (void*)NULL;
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

int newClient(char* userid){
	int i;
	for(i = 0; i < MAX_CLIENTS; i++){
		if(!clients[i].logged_in){
                        clients[i].devices[0] = 1;
			strcpy(clients[i].userid, userid);
			char server_new_client_folder[2*MAXNAME +1];
	   		sprintf(server_new_client_folder, "%s/%s", serverInfo.folder, userid);
			clients[i].n_files = get_dir_content_file_info(server_new_client_folder, clients[i].file_info); 
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

int removeDevice(Client* client, int device){
	if(client){
		client->devices[device] = 0;
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


