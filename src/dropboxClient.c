#include "dropboxClient.h"

pthread_t sync_thread;
struct user_info user;
int sockid;
int status;

char buffer[BUFFER_SIZE];

void clearBuffer() {
  int i;
  for(i = 0; i < BUFFER_SIZE; i++){
    buffer[i] = 0;
  }
}

int connect_server (char *host, int port) {

  struct sockaddr_in serverconn;

  /* Create a socket point */
  sockid = socket(AF_INET, SOCK_STREAM, 0);

  if (sockid < 0) {
      printf("ERROR opening socket\n");
      exit(1);
  }

  bzero((char *) &serverconn, sizeof(serverconn));

  serverconn.sin_family = AF_INET;
  serverconn.sin_port = htons(port);
  serverconn.sin_addr.s_addr = inet_addr(host);



  status = connect(sockid, (struct sockaddr *) &serverconn, sizeof(serverconn));
  if(status < 0){
    printf("\nConnection Error\n");
    exit(1);
  }

  bzero(buffer, BUFFER_SIZE);

  strcpy(buffer, user.id);

  // write to socket
  status = write(sockid, buffer, BUFFER_SIZE);
  if (status < 0) {
      printf("ERROR writing to socket\n");
      exit(1);
   }

  // read server response
  bzero(buffer, BUFFER_SIZE);
  status = read(sockid, buffer, BUFFER_SIZE);
  if (status < 0) {
     printf("ERROR reading from socket\n");
     exit(1);
  }
  if(strcmp(buffer, S_CONNECTED) == 0){
    printf("\nConnected\n");
    return 1;
  }
  if(strcmp(buffer, S_EXCESS_DEVICES) == 0){
    printf("\nMuitas conexões simultaneas do mesmo usuário. encerrando...\n");
  }

  return 0;
}

void sync_client() {
	// verifica se o diretório sync_dir_<nomeusuario> existe
	// se não, cria pasta e fecha.
	if(!fileExists(user.folder)) {
		if(mkdir(user.folder, 0777) != 0) {
			printf("Error creating user folder '%s'.\n", user.folder);
		}

		return;
	}

	// sincroniza pasta local com o servidor
	// cria thread para manter a sincronização
	int rc;
	if((rc = pthread_create(&sync_thread, NULL, watcher_thread, (void *) user.folder))) {
		printf("Syncronization Thread creation failed: %d\n", rc);
	}

	// se há um conflito entre cópias do mesmo arquivo entre cliente e servidor,
	// apagar arquivo local e trazer do servidor.
		// delete_file, mas apenas local.
		// get_file
}

void send_file(char *file) {
	int file_size = 0;
	int bytes_read = 0;
	int status = 0;

	strcpy(buffer, S_UPLOAD);
	status = write(sockid, buffer, BUFFER_SIZE); // requisita upload

	status = read(sockid, buffer, BUFFER_SIZE); // recebe resposta

	if(strcmp(buffer, S_NAME) == 0){ // envia o nome do arquivo para o servidor
	   strcpy(buffer, file);
	   status = write(sockid, buffer, BUFFER_SIZE);
	}

	FILE* pFile;
	char buffer[BUFFER_SIZE]; // 1 KB buffer

	pFile = fopen(file, "rb");
	if(pFile) {
		file_size = getFilesize(pFile);

		sprintf(buffer, "%d", file_size); // envia tamanho do arquivo para o servidor
		status = write(sockid, buffer, BUFFER_SIZE);

		if(file_size == 0) {
			fclose(pFile);
			return;
		}

		while(!feof(pFile)) {
				fread(buffer, sizeof(char), BUFFER_SIZE, pFile);
				bytes_read += sizeof(char) * BUFFER_SIZE;

				// enviar buffer para salvar no servidor
				status = write(sockid, buffer, BUFFER_SIZE);
        			//status = read(sockid, buffer, BUFFER_SIZE);
				//printf("recebido: %s", buffer);
		}

		fclose(pFile);

    		status = write(sockid, buffer, BUFFER_SIZE);

		printf("Arquivo %s enviado.\n", file);
	} else {
		printf("Erro abrindo arquivo %s.\n", file);
	}
}

void get_file(char *file) {
	int bytes_written = 0;
        int file_size = 0;


        strcpy(buffer, S_DOWNLOAD);
	status = write(sockid, buffer, BUFFER_SIZE); // requisita download

	status = read(sockid, buffer, BUFFER_SIZE); // recebe resposta

	if(strcmp(buffer, S_NAME) == 0){ // envia o nome do arquivo para o servidor
	   printf("envia...\n");
	   strcpy(buffer, file);
	   status = write(sockid, buffer, BUFFER_SIZE);
	}
        printf("nome: %s\n", file);

        char filename_path[MAXNAME*2];
	sprintf(filename_path, "%s/%s", user.folder, file);
	printf("%s\n", filename_path);

	FILE* pFile;
	char buffer[BUFFER_SIZE]; // 1 KB buffer

	pFile = fopen(filename_path, "wb");
	if(pFile) {
                status = read(sockid, buffer, BUFFER_SIZE); // recebe tamanho do arquivo
                file_size = atoi(buffer);
		printf("tamanho: %d\n", file_size);

		status = 0;
		int bytes_to_read = file_size;
		while(file_size > bytes_written) {
			status = read(sockid, buffer, BUFFER_SIZE); // le no buffer
			if(bytes_to_read > BUFFER_SIZE){ // se o tamanho do arquivo for maior, lê buffer completo
				fwrite(buffer, sizeof(char), BUFFER_SIZE, pFile);
				bytes_written += sizeof(char) * BUFFER_SIZE;
				bytes_to_read -= bytes_to_read;
			 } else{ // senão lê só o bytes_to_read
				fwrite(buffer, sizeof(char), bytes_to_read, pFile);
				bytes_written += sizeof(char) * bytes_to_read;
			 }
			      printf("leu\n");
		 }

		fclose(pFile);

		printf("Arquivo %s salvo.\n", filename_path);
	} else {
		printf("Erro abrindo arquivo %s.\n", filename_path);
	}
}

void delete_file(char *file) {
	// avisa servidor para remover arquivo file
	// recebe confirmação de que arquivo foi removido
}

void close_connection() {
	// Fechar a thread de sincronização
	pthread_cancel(sync_thread);

	// Fechar conexão com o servidor
	bzero(buffer, BUFFER_SIZE);

	strcpy(buffer, S_REQ_DC);
	status = write(sockid, buffer, BUFFER_SIZE);
  	if (status < 0) {
      		printf("ERROR writing to socket\n");
      		exit(1);
   	}

  	status = read(sockid, buffer, BUFFER_SIZE);
  	if (status < 0) {
     		printf("ERROR reading from socket\n");
     		exit(1);
  	}

  	if(strcmp(buffer, S_RPL_DC) == 0){
    		printf("\nDesconectado!\n");
		close(sockid);
  	} else{
		printf("Erro ao desconectar!\n");
	}



}

void list_server(){
	int number_files = 0;

	strcpy(buffer, S_LS);
	status = write(sockid, buffer, BUFFER_SIZE); // requisita list server

  	// Lista os arquivos salvos no servidor associados ao usuário.

  	status = read(sockid, buffer, BUFFER_SIZE); // numero de arquivos
	number_files = atoi(buffer);
	printf("%d\n", number_files);
	for(int i = 0; i < number_files; i++){
		status = read(sockid, buffer, BUFFER_SIZE);
		printf("%s\n", buffer);
	}
}

int main(int argc, char *argv[]) {
	if (argc < 3) {
		puts("Argumentos Insuficientes.");
		puts("Formato esperado: './dropboxClient user endereço porta'");

		exit(0);
	}

	int porta;
	char *endereco;

	// Parsing de argumentos do programa
	if (strlen(argv[1]) <= MAXNAME) {
		strcpy(user.id, argv[1]);
		sprintf(user.folder, "%s/sync_dir_%s", getUserHome(), user.id);
	} else {
		puts("Tamanho máximo de userid foi ultrapassado.");
		printf("Máximo: %d. Inserido: %d.\n", MAXNAME, strlen(argv[1]) <= MAXNAME);

		return MAXNAMESIZE_REACHED;
	}

	endereco = malloc(sizeof(argv[2]));
	strcpy(endereco, argv[2]);

	porta = atoi(argv[3]);

	// Efetua conexão ao servidor
	if ((connect_server(endereco, porta))) {
		// sincroniza diretórios (cliente e servidor)
		sync_client();

		// cria a interface do cliente e espera por comandos
		show_client_interface();
	} else {
    		printf("Conexão ao servidor '%s' na porta '%d' falhou.\n", endereco, porta);
		//close(sockid);
	}
}
