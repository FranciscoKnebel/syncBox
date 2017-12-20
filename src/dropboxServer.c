#include "dropboxServer.h"

ServerInfo serverInfo;
ClientList* client_list;
ClientList* client_list_servers;
int numServers = 0;
pthread_mutex_t mutex_clientes;
pthread_mutex_t mutex_clientes_servers;

pthread_t thread_replica;

sem_t semaphore;

char addressToConnect[MAXNAME];
int portToConnect = 0;

int porta;
char address[MAXNAME];

char replicaHost[MAXNAME];
int replicaPort;

int was_replica = 0;

int configLine = 1;
Connection *connection_replica;

void* connect_server_replica (void* connection_struct) {
      DEBUG_PRINT("Inicia conexão replica\n");

      Connection* connection = (Connection*) connection_struct;
      char* host = connection->ip;
      int port = connection->porta;


      struct sockaddr_in serverconn;
      SSL *ssl;
      SSL_CTX	*ctx;
      const SSL_METHOD *method;
      int sockid;
      char replicaName[MAXNAME];
      char buffer[BUFFER_SIZE];

    	OpenSSL_add_all_algorithms();
    	SSL_load_error_strings();
    	method	=	SSLv23_client_method();
    	ctx	=	SSL_CTX_new(method);
    	if(ctx	==	NULL) {
    		ERR_print_errors_fp(stderr);
    		abort();
    	}
    	DEBUG_PRINT("Inicializado a engine SSL\n");

    	/* Create a socket point */
    	sockid = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    	if (sockid < 0) {
    		printf("ERROR opening socket\n");
    		return 0;
    	}

    	DEBUG_PRINT("Criou socket\n");

    	bzero((char *) &serverconn, sizeof(serverconn));

    	serverconn.sin_family = AF_INET;
    	serverconn.sin_port = htons(port);
    	serverconn.sin_addr.s_addr = inet_addr(host);

    	if (connect(sockid, (struct sockaddr*) &serverconn, sizeof(serverconn)) != 0){
    		close(sockid);
    		perror(host);
    		abort();
    	}

    	DEBUG_PRINT("Conectou socket\n");

    	ssl	=	SSL_new(ctx);
    	SSL_set_fd(ssl,	sockid);

    	DEBUG_PRINT("SSL anexado ao socket\n");

    	if(SSL_connect(ssl)	== -1) {
    		DEBUG_PRINT("Erro no ssl_connect\n");
    		ERR_print_errors_fp(stderr);
    	} else {	// conexão aceita
    		DEBUG_PRINT("Conectou o ssl\n");
    		printf("Conexão com criptografia %s estabelecida.\n", SSL_get_cipher(ssl));

    		ShowCerts(ssl);
    		bzero(buffer, BUFFER_SIZE);
    		strcpy(buffer, S_SERVER_REPLICA);

    		// write to socket
    		write_to_socket(ssl, buffer); // envia S_SERVER_REPLICA

        read_from_socket(ssl, buffer); // recebe nome de identificaçao desse server replica no principal
        strcpy(replicaName, buffer);

        strcpy(buffer, address);
        write_to_socket(ssl, buffer);
        sprintf(buffer, "%d", porta);
        write_to_socket(ssl, buffer);


        synchronize_replica_receive(ssl, serverInfo.folder);

        char filePath_server[MAXNAME*2];
        char last_modified[MAXNAME];
        char filePath_local[MAXNAME*2];
        char filename[MAXNAME];

        while(1){
          if(read_from_socket(ssl, buffer) == 0){ // recebe S_UPLOAD ou S_DELETE ou S_NEW_FOLDER ou nao recebe nada (server caiu)
            reconnect_server_replica();
          } else{
            if(strcmp(buffer, S_UPLOAD) == 0){
              read_from_socket(ssl, buffer); // nome do arquivo no server
          		strcpy(filePath_server, buffer);
              DEBUG_PRINT("connect_server_replica: Path recebido: %s\n", filePath_server);
              sprintf(filename, "%s", buffer);
              DEBUG_PRINT("connect_server_replica:Filename a receber: %s\n", filename);
              // recebe o arquivo
              sprintf(filePath_local, "%s/%s", serverInfo.folder, filename);
              DEBUG_PRINT("connect_server_replica: Recebendo: %s\n", filePath_local);
              read_from_socket(ssl, buffer); // timestamp
              strcpy(last_modified, buffer);
              DEBUG_PRINT("connect_server_replica: Last modified recebido: %s\n", last_modified);
              receive_file(filePath_local, ssl);
              time_t last_modified_time = getTime(last_modified);
              setModTime(filePath_local, last_modified_time);
            } else if(strcmp(buffer, S_DELETE) == 0){
              read_from_socket(ssl, buffer); // nome do arquivo no server
              strcpy(filePath_server, buffer);
              DEBUG_PRINT("connect_server_replica: Path recebido: %s\n", filePath_server);
              sprintf(filename, "%s", buffer);
              DEBUG_PRINT("connect_server_replica:Filename a deletar: %s\n", filename);
              // recebe o arquivo
              sprintf(filePath_local, "%s/%s", serverInfo.folder, filename);
              if(remove(filePath_local) != 0) {
            		DEBUG_PRINT("Erro ao deletar o arquivo %s\n", filePath_local);
            	} else {
              	DEBUG_PRINT("Arquivo %s excluido!\n", filePath_local);
              }
              } else if(strcmp(buffer, S_NEW_FOLDER) == 0){
                read_from_socket(ssl, buffer);
                DEBUG_PRINT("Criando folder de nome %s", buffer);
                char folderPath[MAXNAME*2];
                sprintf(folderPath, "%s/%s", serverInfo.folder, buffer);
                if(!fileExists(folderPath)) {
                  if(mkdir(folderPath, 0777) != 0) {
                    printf("Error creating user folder in server '%s'.\n", folderPath);
                    return 0;
                  }
                }
              }
          }
    	}
    }
  return 0;
}

int reconnect_server_replica() {
	// config para conexão aos servidores backup
	char filename_config[MAXNAME];
	sprintf(filename_config, "%s/%s", getUserHome(), "connection.config");
	FILE *file_config = fopen(filename_config, "r");
	int count = 1; // inicia na linha 1
	char line[MAXNAME];
	if(file_config == NULL){
		DEBUG_PRINT("Arquivo '%s' nao encontrado\n", filename_config);
	} else{
    configLine+=2;
		while (fgets(line, sizeof line, file_config) != NULL){ // read a line
        DEBUG_PRINT("count: %d, configLine: %d\n", count, configLine);
        if (count == configLine){
						DEBUG_PRINT("config: host lido: %s\n", line);
						strcpy(addressToConnect, line);
						fgets(line, sizeof line, file_config);
						DEBUG_PRINT("config: porta lida: %s\n", line);
						portToConnect = atoi(line);
            count++;
        }
        else{
            count++;
        }
    }
    fclose(file_config);
	}

  if((strcmp(addressToConnect, address) != 0) && (portToConnect != porta)){
      DEBUG_PRINT("\nENDERECO DIFERENTE E PORTA DIFERENTE\n");
      strcpy(connection_replica->ip, addressToConnect);
      connection_replica->porta = portToConnect;

      connect_server_replica((void*) connection_replica);
  }else{
    DEBUG_PRINT("\nENDERECO IGUAL OU PORTA IGUAL!!!!!\n");
    pthread_exit(NULL);
  }

  return 0;
}


void sync_server(SSL *ssl_sync, Client* client_sync) {
  // sincronizar arquivos no dispositivo do cliente
  synchronize_client(ssl_sync, client_sync);

  // sincroniza agora o servidor com os arquivos do cliente
  synchronize_server(ssl_sync, client_sync);

  DEBUG_PRINT("Sincronização cliente/servidor encerrada.\n");
}

void receive_file(char *file, SSL *ssl_download) {
  int bytes_written = 0;
  int file_size = 0;
  char lock_path[MAXNAME*3+1];


  DEBUG_PRINT("receive_file: recebendo %s\n", file);

  FILE* pFile;
  char buffer[BUFFER_SIZE]; // 1 KB buffer
  read_from_socket(ssl_download, buffer); // recebe tamanho do arquivo ou "erro no arquivo"

  if(strcmp(buffer, S_ERRO_ARQUIVO) != 0){
    pFile = fopen(file, "wb");
    if(pFile) {
      file_size = atoi(buffer);
      DEBUG_PRINT("receive_file: tamanho recebido: %d\n", file_size);
      bytes_written = read_to_file(pFile, file_size, ssl_download);

      if(bytes_written == file_size) {
				DEBUG_PRINT("receive_file: Terminou de escrever.\n");
			} else {
				DEBUG_PRINT("receive_file: Erro ao escrever %d bytes. Esperado %d.\n", bytes_written, file_size);
			}
      fclose(pFile);

      DEBUG_PRINT("receive_file: Arquivo '%s%s%s' salvo.\n", COLOR_GREEN, file, COLOR_RESET);
      updateReplicas(file, S_UPLOAD);
    } else {
      DEBUG_PRINT("Erro abrindo arquivo %s.\n", file);
    }
  } else {
    DEBUG_PRINT("Receive \"erro no arquivo\": %s\n", buffer);
    DEBUG_PRINT("Erro abrindo no cliente %s.\n", file);
  }

  sprintf(lock_path, "%s.lock", file);

  if(fileExists(lock_path)) {
    if(remove(lock_path) != 0) {
      DEBUG_PRINT("Erro ao deletar o arquivo LOCK %s\n", lock_path);
    } else {
      DEBUG_PRINT("Arquivo LOCK %s excluido!\n", lock_path);
    }
  }
}

int updateReplicas(char* file_path, char* command){
  ClientList* current = client_list_servers;
  char buffer[BUFFER_SIZE];

  while(current != NULL){
    Client* client = current->client;
    SSL* ssl = client->devices[0];
    char last_modified[MAXNAME];

    if(strcmp(command, S_UPLOAD) == 0){
      strcpy(buffer, S_UPLOAD);
      write_to_socket(ssl,buffer); // envia string upload
      strcpy(buffer, file_path + strlen(serverInfo.folder) + 1);
      DEBUG_PRINT("update replicas: Enviando: %s\n", buffer);
      write_to_socket(ssl, buffer); // envia nome
      DEBUG_PRINT("Nome: %s\n", file_path + strlen(serverInfo.folder) + 1);
      if (strchr(file_path + strlen(serverInfo.folder) + 1, '/') != NULL){
        getFileModifiedTime(file_path, last_modified);
        strcpy(buffer, last_modified);
        DEBUG_PRINT("enviando Last modified: %s\n", buffer);
        write_to_socket(ssl, buffer); // envia last modified
        DEBUG_PRINT("Enviando: %s\n", file_path);
        send_file(file_path, ssl, FALSE);
      } else{
        DEBUG_PRINT("Enviando nome da pasta!\n");
      }
  } else if(strcmp(command, S_DELETE) == 0){
    strcpy(buffer, S_DELETE);
    write_to_socket(ssl,buffer); // envia string upload
    strcpy(buffer, file_path + strlen(serverInfo.folder) + 1);
    DEBUG_PRINT("update replicas: Enviando: %s\n", buffer);
    write_to_socket(ssl, buffer); // envia nome
  } else if(strcmp(command, S_NEW_FOLDER) == 0){
    DEBUG_PRINT("Enviando S_NEW_FOLDER\n");
    strcpy(buffer, S_NEW_FOLDER);
    write_to_socket(ssl, buffer);
    DEBUG_PRINT("File path: %s\n", file_path);
    strcpy(buffer, file_path);
    write_to_socket(ssl, buffer);
    DEBUG_PRINT("nome do folder: %s\n", buffer);
  }
    current = current->next;
  }
  return 0;
}


void send_file(char *file, SSL *ssl_download, int send_mod_time) {
  int file_size = 0;
  int bytes_read = 0;

  FILE* pFile;
  char buffer[BUFFER_SIZE]; // 1 KB buffer

  pFile = fopen(file, "rb");
  if(pFile) {
    file_size = getFilesize(pFile);
    DEBUG_PRINT("file size: %d\n", file_size);
    sprintf(buffer, "%d", file_size);
    write_to_socket(ssl_download, buffer); // envia tamanho do arquivo para o cliente

    if(send_mod_time){
      getFileModifiedTime(file, buffer);
      write_to_socket(ssl_download, buffer); // modified time
    }


    while(!feof(pFile)) {
      fread(buffer, sizeof(char), BUFFER_SIZE, pFile);
      bytes_read += sizeof(char) * BUFFER_SIZE;

      // enviar buffer para salvar no cliente
      write_to_socket(ssl_download, buffer);
    }

    fclose(pFile);
    DEBUG_PRINT("Arquivo %s enviado (%d bytes).\n", file, bytes_read);
  } else {
    DEBUG_PRINT("Erro abrindo arquivo %s.\n", file);
    strcpy(buffer, S_ERRO_ARQUIVO);
    write_to_socket(ssl_download, buffer);
  }
}


void parseArguments(int argc, char *argv[], char* address, int* port, char* addressReplica, int* portReplica) {
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
  if(argc > 3){
    strcpy(addressReplica, argv[3]);
    *portReplica = atoi(argv[4]);
  }
}

void* clientThread(void* connection_struct) {
  Connection* connection = (Connection*) connection_struct;
  char buffer[BUFFER_SIZE];

  SSL *ssl = connection->ssl;
  char* client_ip = connection->ip;
  char client_id[MAXNAME];
  int device = 0;
  Client* client;

  bzero(buffer, BUFFER_SIZE);

  // read
  read_from_socket(ssl, buffer); // le o nome do cliente conectado

  if(strcmp(buffer, S_SERVER_REPLICA) == 0){ // SERVIDOR REPLICA QUE CONECTOU!
    pthread_mutex_lock(&mutex_clientes_servers);
    numServers += 1;
    sprintf(client_id, "%s%d", buffer, numServers);
    sprintf(buffer, client_id, numServers);
    write_to_socket(ssl, buffer);
    client_list_servers = newClient(client_id, ssl, client_list_servers);
    DEBUG_PRINT("Adicionado %s a lista de servidores replica\n", client_id);
    pthread_mutex_unlock(&mutex_clientes_servers);

    client = searchClient(client_id, client_list_servers);

    read_from_socket(ssl, buffer);
    strcpy(client->host, buffer);
    read_from_socket(ssl, buffer);
    client->port = atoi(buffer);

    DEBUG_PRINT("host: %s porta: %d\n", client->host, client->port);

    // Sincroniza as pastas dos servidores
    synchronize_replica_send(ssl, client_list, serverInfo.folder);

    while(1){
      /*strcpy(buffer, ".");
      write_to_socket(ssl, buffer);
      usleep(1000);
      */
    }
  } else{

    strncpy(client_id, buffer, MAXNAME);
    client_id[MAXNAME - 1] = '\0';

    strcpy(buffer, S_CONNECTED);
    DEBUG_PRINT("Cliente conectado: '%s%s%s'.\n", COLOR_GREEN, client_id, COLOR_RESET);
    client = searchClient(client_id, client_list);
    DEBUG_PRINT("Cliente '%s%s%s' encontrado: %s.\n",
    COLOR_GREEN, client_id, COLOR_RESET,
    client == NULL ? "FALSE" : "TRUE");

    if(client == NULL) {
      DEBUG_PRINT("Criando novo cliente.\n");
      pthread_mutex_lock(&mutex_clientes);
      client_list = newClient(client_id, ssl, client_list);
      pthread_mutex_unlock(&mutex_clientes);
      updateReplicas(client_id, S_NEW_FOLDER);
      client = searchClient(client_id, client_list);
    } else {
      DEBUG_PRINT("Adicionando device ao cliente encontrado.\n");
      pthread_mutex_lock(&mutex_clientes);
      device = addDevice(client, ssl);
      pthread_mutex_unlock(&mutex_clientes);
    }
    DEBUG_PRINT("device: %d\n", device);

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


      write_to_socket(ssl, buffer); // envia "connected"

      if(!was_replica){
        sync_server(ssl, client);
      }

      int disconnected = 0;
      do {
        read_from_socket(ssl, buffer);

        if(strcmp(buffer, S_REQ_DC) == 0) {
          strcpy(buffer, S_RPL_DC);
          write_to_socket(ssl, buffer);

          sem_post(&semaphore);

          disconnected = 1;
        } else if(strcmp(buffer, ".") == 0){
            DEBUG_PRINT("Check de conexão!\n");
            sprintf(buffer, "%s", S_CONNECTED);
            write_to_socket(ssl, buffer);
            DEBUG_PRINT("Escrito S_CONNECTED no buffer\n");
        } else if(is_valid_command(buffer)) {
          DEBUG_PRINT("Comando do usuário: %s\n", buffer);

          select_commands(ssl, buffer, client);
        } else {
          DEBUG_PRINT("Comando inserido é inválido.\n");
          DEBUG_PRINT("Informado: \n%s\n", buffer);
        }
      } while(disconnected != 1);

      pthread_mutex_lock(&mutex_clientes);
      printf("'%s%s%s' desconectou no dispositivo '%s%d%s'!\n",
      COLOR_GREEN, client_id, COLOR_RESET,
      COLOR_GREEN, removeDevice(client, device, client_list), COLOR_RESET);
      client_list = check_login_status(client, client_list);
      pthread_mutex_unlock(&mutex_clientes);

    } else {
      DEBUG_PRINT("Muitas conexões simultâneas de '%s%s%s' em '%s%s%s'. Acesso negado.\n",
      COLOR_GREEN, client_id, COLOR_RESET,
      COLOR_GREEN, client_ip, COLOR_RESET);

      strcpy(buffer, S_EXCESS_DEVICES);
      write_to_socket(ssl, buffer);
    }
  }
  return 0;
}

int main(int argc, char *argv[]) { // ./dropboxServer endereço porta
  setlocale(LC_ALL, "pt_BR");
  porta = DEFAULT_PORT;
  struct sockaddr_in server, client;
  int isReplica = 0;
  replicaPort = DEFAULT_PORT;

  pthread_mutex_init (&mutex_clientes, NULL); // inicializa mutex da fila de clientes
  pthread_mutex_init (&mutex_exclusao_mutua_lock, NULL);

  sem_init(&semaphore, 0, MAX_CLIENTS);

  //int addressLengthReplica = (argc > 3) ? strlen(argv[3]) : strlen(DEFAULT_ADDRESS);
  //replicaHost = malloc(addressLengthReplica + 1);

  /* Initialize socket structure */
  bzero((char *) &server, sizeof(server));

  parseArguments(argc, argv, address, &porta, replicaHost, &replicaPort);

  if(argc > 3){ // é replica
    isReplica = 1;
    DEBUG_PRINT("É Replica! - host: %s, porta: %d\n", replicaHost, replicaPort);
  } else{
    DEBUG_PRINT("É servidor principal!\n");
  }
  server.sin_family = AF_INET; // address format is host and port number
  server.sin_port = htons(porta); // host to network short
  server.sin_addr.s_addr = inet_addr(address);

  sprintf(serverInfo.folder, "%s/%s", getUserHome(), SERVER_FOLDER);
  strcpy(serverInfo.ip, address);
  serverInfo.port = porta;

  // ssl
  SSL_library_init();
  const SSL_METHOD *method;
	SSL_CTX	*ctx;

	OpenSSL_add_all_algorithms();  /* Load cryptos, et.al. */
	SSL_load_error_strings();
	method = SSLv23_server_method();
	ctx	=	SSL_CTX_new(method);
	if (ctx	== NULL) {
		ERR_print_errors_fp(stderr);
		abort();
	}

  // Criação e nomeação de socket
  int sockid = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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

  if(isReplica){
    was_replica = 1;
    pthread_t thread_replica;
    connection_replica = malloc(sizeof(*connection_replica));
    connection_replica->ip = malloc(sizeof(MAXNAME));
    strcpy(connection_replica->ip, replicaHost);
    connection_replica->porta = replicaPort;

    if(pthread_create(&thread_replica, NULL, connect_server_replica, (void*) connection_replica) < 0){
      printf("Error on create thread\n");
    }
  }


  pthread_t thread_id;
  while(1) {
    unsigned int cliLen = sizeof(struct sockaddr_in);

    LoadCertificates(ctx, "CertFile.pem", "KeyFile.pem");

    int new_client_socket = accept(sockid, (struct sockaddr *) &client, &cliLen);
    if(new_client_socket < 0){
      printf("Error on accept\n");
    }
    SSL *ssl = SSL_new(ctx);
		SSL_set_fd(ssl,	new_client_socket);

    if (SSL_accept(ssl) == -1) {     /* do SSL-protocol accept */
      ERR_print_errors_fp(stderr);
    } else {
      sem_wait(&semaphore);

      char *client_ip = inet_ntoa(client.sin_addr); // inet_ntoa converte o IP de numeros e pontos para uma struct in_addr

      Connection *connection = malloc(sizeof(*connection));
      connection->socket_id = new_client_socket;
      connection->ip = client_ip;
      connection->ssl = ssl;

      if(pthread_create(&thread_id, NULL, clientThread, (void*) connection) < 0){
        printf("Error on create thread\n");
      }
    }
  }

  return 0;

}
