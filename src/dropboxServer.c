#include "dropboxServer.h"

ServerInfo serverInfo;
ClientList* client_list;
pthread_mutex_t mutex_clientes;

sem_t semaphore;


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


  DEBUG_PRINT("recebendo %s\n", file);

  FILE* pFile;
  char buffer[BUFFER_SIZE]; // 1 KB buffer
  read_from_socket(ssl_download, buffer); // recebe tamanho do arquivo ou "erro no arquivo"

  if(strcmp(buffer, S_ERRO_ARQUIVO) != 0){
    pFile = fopen(file, "wb");
    if(pFile) {
      file_size = atoi(buffer);

      bytes_written = read_to_file(pFile, file_size, ssl_download);
      if(bytes_written == file_size) {
				DEBUG_PRINT("Terminou de escrever.\n");
			} else {
				DEBUG_PRINT("Erro ao escrever %d bytes. Esperado %d.\n", bytes_written, file_size);
			}
      fclose(pFile);

      DEBUG_PRINT("Arquivo '%s%s%s' salvo.\n", COLOR_GREEN, file, COLOR_RESET);
    } else {
      DEBUG_PRINT("Erro abrindo arquivo %s.\n", file);
    }
  } else {
    DEBUG_PRINT("Receive \"erro no arquivo\": %s\n", buffer);
    DEBUG_PRINT("Erro abrindo no cliente %s.\n", file);
  }

  sprintf(lock_path, "%s.lock", file);

  if(fileExists(lock_path)) {
    //usleep(5000000); só para teste...
    if(remove(lock_path) != 0) {
      DEBUG_PRINT("Erro ao deletar o arquivo LOCK %s\n", lock_path);
    } else {
      DEBUG_PRINT("Arquivo LOCK %s excluido!\n", lock_path);
    }
  }
}

void send_file(char *file, SSL *ssl_download) {
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

    getFileModifiedTime(file, buffer);
    write_to_socket(ssl_download, buffer); // modified time

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

void* clientThread(void* connection_struct) {
  Connection* connection = (Connection*) connection_struct;
  char buffer[BUFFER_SIZE];

  SSL *ssl = connection->ssl;
  int socket = connection->socket_id;
  char* client_ip = connection->ip;
  char client_id[MAXNAME];
  int device = 0;
  Client* client;

  bzero(buffer, BUFFER_SIZE);

  // read
  read_from_socket(ssl, buffer);
  // do stuff...
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
    client_list = newClient(client_id, socket, client_list);
    pthread_mutex_unlock(&mutex_clientes);
    client = searchClient(client_id, client_list);
  } else {
    DEBUG_PRINT("Adicionando device ao cliente encontrado.\n");
    pthread_mutex_lock(&mutex_clientes);
    device = addDevice(client, socket);
    pthread_mutex_unlock(&mutex_clientes);
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

    write_to_socket(ssl, buffer);

    sync_server(ssl, client);

    int disconnected = 0;
    do {
      read_from_socket(ssl, buffer);

      if(strcmp(buffer, S_REQ_DC) == 0) {
        strcpy(buffer, S_RPL_DC);
        write_to_socket(ssl, buffer);

        sem_post(&semaphore);

        disconnected = 1;
      } else if(is_valid_command(buffer)) {
        DEBUG_PRINT("Comando do usuário: %s\n", buffer);

        select_commands(ssl, buffer, client);
      } else {
        DEBUG_PRINT("Comando inserido é inválido.\n");
        DEBUG_PRINT("Informado: \n%s\n", buffer);
      }
    } while(disconnected != 1);

    pthread_mutex_lock(&mutex_clientes);
    printf("'%s%s%s' desconectou no dispositivo '%s%d%s', socket '%s%d%s'!\n",
    COLOR_GREEN, client_id, COLOR_RESET,
    COLOR_GREEN, removeDevice(client, device, client_list), COLOR_RESET,
    COLOR_GREEN, socket, COLOR_RESET);
    client_list = check_login_status(client, client_list);
    pthread_mutex_unlock(&mutex_clientes);

  } else {
    DEBUG_PRINT("Muitas conexões simultâneas de '%s%s%s' em '%s%s%s'. Acesso negado.\n",
    COLOR_GREEN, client_id, COLOR_RESET,
    COLOR_GREEN, client_ip, COLOR_RESET);

    strcpy(buffer, S_EXCESS_DEVICES);
    write_to_socket(ssl, buffer);
  }
  return 0;
}

void LoadCertificates(SSL_CTX* ctx, char* CertFile, char* KeyFile){
    /* set the local certificate from CertFile */
    if (SSL_CTX_use_certificate_file(ctx, CertFile, SSL_FILETYPE_PEM) <= 0 )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    /* set the private key from KeyFile (may be the same as CertFile) */
    if (SSL_CTX_use_PrivateKey_file(ctx, KeyFile, SSL_FILETYPE_PEM) <= 0 )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    /* verify private key */
    if (!SSL_CTX_check_private_key(ctx) )
    {
        fprintf(stderr, "Private key does not match the public certificate\n");
        abort();
    }
}

void ShowCerts(SSL* ssl)
{   X509 *cert;
    char *line;

    cert = SSL_get_peer_certificate(ssl); /* Get certificates (if available) */
    if ( cert != NULL )
    {
        printf("Server certificates:\n");
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        printf("Subject: %s\n", line);
        free(line);
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        printf("Issuer: %s\n", line);
        free(line);
        X509_free(cert);
    }
    else{
        printf("No certificates.\n");
    }
}

int main(int argc, char *argv[]) { // ./dropboxServer endereço porta
  setlocale(LC_ALL, "pt_BR");
  int port = DEFAULT_PORT;
  struct sockaddr_in server, client;

  pthread_mutex_init (&mutex_clientes, NULL); // inicializa mutex da fila de clientes
  pthread_mutex_init (&mutex_exclusao_mutua_lock, NULL);

  sem_init(&semaphore, 0, MAX_CLIENTS);

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

  SSL_library_init();
  // ssl
  const SSL_METHOD	*method;
	SSL_CTX	*ctx;

	OpenSSL_add_all_algorithms();  /* Load cryptos, et.al. */
	SSL_load_error_strings();
	method = SSLv23_server_method();
	ctx	=	SSL_CTX_new(method);
	if	(ctx	==	NULL){
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

  pthread_t thread_id;

  while(1) {
    unsigned int cliLen = sizeof(struct sockaddr_in);

    LoadCertificates(ctx, "CertFile.pem", "KeyFile.pem");

    int new_client_socket = accept(sockid, (struct sockaddr *) &client, &cliLen);
    if(new_client_socket < 0){
      printf("Error on accept\n");
    }
    SSL *ssl	=	SSL_new(ctx);
		SSL_set_fd(ssl,	new_client_socket);

    if ( SSL_accept(ssl) == -1){     /* do SSL-protocol accept */
        ERR_print_errors_fp(stderr);
    } else{
        ShowCerts(ssl);

        //DEBUG_PRINT("semaforo: %d\n", sem_wait(&semaphore)); // zancan, não dá pra deixar dentro do debug código. Se tirar do debug, isso não vai executar
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
