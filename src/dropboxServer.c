#include "dropboxServer.h"

ServerInfo serverInfo;
ClientList* client_list;
char buffer[BUFFER_SIZE];

void sync_server(int sockid_sync, Client* client_sync) {
  // sincronizar arquivos no dispositivo do cliente
  syncronize_client(sockid_sync, client_sync);

  // sincroniza agora o servidor com os arquivos do cliente
  syncronize_server(sockid_sync, client_sync);

  DEBUG_PRINT("sincronizacao finalizada!\n");
}

void receive_file(char *file, int sockid_upload) {
  int bytes_written = 0;
  int status = 0;
  int file_size = 0;

  DEBUG_PRINT("recebendo %s\n", file);

  FILE* pFile;
  char buffer[BUFFER_SIZE]; // 1 KB buffer

  pFile = fopen(file, "wb");
  if(pFile) {
    // requisita o arquivo file do cliente
    // recebe buffer do cliente
    status = read(sockid_upload, buffer, BUFFER_SIZE);
    if (status < 0) {
      printf("ERROR reading from socket\n");
    }
    file_size = atoi(buffer);

    bytes_written = 0;
    while(file_size > bytes_written) {
      status = read(sockid_upload, buffer, BUFFER_SIZE); // le no buffer
      if (status < 0) {
        printf("ERROR reading from socket\n");
      }

      if((file_size - bytes_written) > BUFFER_SIZE) { // se o tamanho faltando for maior do que o buffer, lê apenas buffer
        fwrite(buffer, sizeof(char), BUFFER_SIZE, pFile);
        bytes_written += sizeof(char) * BUFFER_SIZE;
      } else { // senão lê os bytes que sobram
        fwrite(buffer, sizeof(char), (file_size - bytes_written), pFile);
        bytes_written += sizeof(char) * (file_size - bytes_written);
      }
      DEBUG_PRINT("leu buffer - Total: %d / Escritos: %d / Sobrando: %d\n", file_size, bytes_written, (file_size - bytes_written));
    }
    DEBUG_PRINT("Terminou de escrever.\n");
    fclose(pFile);

    DEBUG_PRINT("Arquivo %s salvo.\n", file);
  } else {
    DEBUG_PRINT("Erro abrindo arquivo %s.\n", file);
  }
}

void send_file(char *file, int sockid_download) {
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
    if (status < 0) {
      DEBUG_PRINT("ERROR writing to socket\n");
    }

    if(file_size == 0) {
      fclose(pFile);
      return;
    }

    while(!feof(pFile)) {
      fread(buffer, sizeof(char), BUFFER_SIZE, pFile);
      bytes_read += sizeof(char) * BUFFER_SIZE;

      // enviar buffer para salvar no cliente
      status = write(sockid_download, buffer, BUFFER_SIZE);
      if (status < 0) {
        DEBUG_PRINT("ERROR writing to socket\n");
	    }
    }

    fclose(pFile);
    DEBUG_PRINT("Arquivo %s enviado.\n", file);
  } else {
    DEBUG_PRINT("Erro abrindo arquivo %s.\n", file);
  }
}

void parseArguments(int argc, char *argv[], char* address, int* port) {
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

void* continueClientProcess(void* connection_struct) {
  Connection* connection = (Connection*) connection_struct;
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
  client_id[MAXNAME - 1] = '\0';

  strcpy(buffer, S_CONNECTED);

  client = searchClient(client_id, client_list);

  if(client == NULL) {
    client = newClient(client_id, socket, client_list);
  } else {
    device = addDevice(client, socket);

    if(device == -1) {
      strcpy(buffer, S_EXCESS_DEVICES);
    }
  }

  DEBUG_PRINT("socket: %d - device: %d\n", socket, device);

  if(device != -1) {
    char client_folder[2*MAXNAME +1];

    sprintf(client_folder, "%s/%s", serverInfo.folder, client_id);
    if(!fileExists(client_folder)) {
      if(mkdir(client_folder, 0777) != 0) {
        printf("Error creating user folder in server '%s'.\n", client_folder);
        return NULL;
      }
    }

    printf("Conexão iniciada do usuário '%s%s%s' através do IP '%s%s%s'.\n",
    COLOR_GREEN, client_id, COLOR_RESET, COLOR_GREEN, client_ip, COLOR_RESET);

    status = write(socket, buffer, BUFFER_SIZE);

    sync_server(socket, client);

    if (status < 0) {
      DEBUG_PRINT("ERROR writing to socket\n");
      exit(1);
    }

    int disconnected = 0;

    bzero(buffer, BUFFER_SIZE);

    do {

      // read
      status = read(socket, buffer, BUFFER_SIZE);
      if (status < 0) {
        DEBUG_PRINT("ERROR reading from socket");
        exit(1);
      }
      DEBUG_PRINT("Lido: %s\n", buffer);

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
	//DEBUG_PRINT("Select commands \n");
        select_commands(socket, buffer, client);
      }
    } while(disconnected != 1);

    printf("'%s%s%s' desconectou no dispositivo '%s%d%s', socket '%s%d%s'!\n",
    COLOR_GREEN, client_id, COLOR_RESET,
    COLOR_GREEN, removeDevice(client, device, client_list), COLOR_RESET,
    COLOR_GREEN, socket, COLOR_RESET);
  } else {
    // write
    status = write(socket, buffer, BUFFER_SIZE);
    if (status < 0) {
      DEBUG_PRINT("ERROR writing to socket\n");
      exit(1);
    }
  }
  return 0;
}

int main(int argc, char *argv[]){ // ./dropboxServer endereço porta
  init_client_list(client_list);
  int port = DEFAULT_PORT;
  struct sockaddr_in server, client;

  int addressLength = (argc > 1) ? strlen(argv[1]) : strlen(DEFAULT_ADDRESS);
  char* address = malloc(addressLength + 1);

  /* Initialize socket structure */
  bzero((char *) &server, sizeof(server));

  parseArguments(argc, argv, address, &port);
  server.sin_family = AF_INET; // address format is host and port number
  server.sin_port = htons(port); // host to network short
  server.sin_addr.s_addr = inet_addr(address);

  sprintf(serverInfo.folder, "%s/%s", getUserHome(), SERVER_FOLDER);
  strcpy(serverInfo.ip, address);
  serverInfo.port = port;

  // Criação e nomeação de socket
  int sockid = socket(AF_INET, SOCK_STREAM, 0);
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

    if(pthread_create(&thread_id, NULL, continueClientProcess, (void*) connection) < 0){
      printf("Error on create thread\n");
    }
  }

  return 0;
}
