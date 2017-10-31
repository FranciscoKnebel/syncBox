#include "dropboxClient.h"

char buffer[BUFFER_SIZE];
pthread_t sync_thread;
struct user_info user;
int sockid;
int status;

int connect_server (char *host, int port) {
	struct sockaddr_in serverconn;
	/* Create a socket point */
	sockid = socket(AF_INET, SOCK_STREAM, 0);
	if (sockid < 0) {
		printf("ERROR opening socket\n");
		return 1;
	}

	bzero((char *) &serverconn, sizeof(serverconn));

	serverconn.sin_family = AF_INET;
	serverconn.sin_port = htons(port);
	serverconn.sin_addr.s_addr = inet_addr(host);

	status = connect(sockid, (struct sockaddr *) &serverconn, sizeof(serverconn));
	if(status < 0){
		printf("\nConnection Error\n");
		return 1;
	}

	bzero(buffer, BUFFER_SIZE-1);
	strcpy(buffer, user.id);

	// write to socket
	status = write(sockid, buffer, BUFFER_SIZE);
	if (status < 0) {
		printf("ERROR writing to socket\n");
		return 1;
 	}

	// read server response
	bzero(buffer, BUFFER_SIZE);
	status = read(sockid, buffer, BUFFER_SIZE);
	if (status < 0) {
		printf("ERROR reading from socket\n");
		return 1;
	}

	if(strcmp(buffer, S_EXCESS_DEVICES) == 0){
		printf("Muitas conexões simultâneas do mesmo usuário.\n");
	}

	if(strcmp(buffer, S_CONNECTED) == 0){
		return 1;
	}

	return 0;
}

void close_connection() {
	// Fechar a thread de sincronização
	pthread_cancel(sync_thread);

	// Fechar conexão com o servidor
	bzero(buffer, BUFFER_SIZE);

	strcpy(buffer, S_REQ_DC);
	status = write(sockid, buffer, BUFFER_SIZE);
	if (status < 0) {
		DEBUG_PRINT("ERROR writing to socket\n");
	}

	status = read(sockid, buffer, BUFFER_SIZE);
	if (status < 0) {
		DEBUG_PRINT("ERROR reading from socket\n");
	}

	if(strcmp(buffer, S_RPL_DC) == 0) {
		DEBUG_PRINT("Desconectado!\n");
		close(sockid);
	} else {
		printf("Erro ao desconectar!\n");
	}
}

void sync_client() {
	// verifica se o diretório sync_dir_<nomeusuario> existe
	// se não, cria pasta.
	if(!fileExists(user.folder)) {
		if(mkdir(user.folder, 0777) != 0) {
			printf("Error creating user folder '%s'.\n", user.folder);
		}
	}

	// sincroniza pasta local com o servidor
	syncronize_local(sockid);

	// sincroniza servidor com pasta local
	syncronize_server(sockid);

	// cria thread para manter a sincronização local
	int rc;
	if((rc = pthread_create(&sync_thread, NULL, watcher_thread, (void *) user.folder))) {
		printf("Syncronization Thread creation failed: %d\n", rc);
	}
}

void send_file(char *file, int response) {
	int file_size = 0;
	int bytes_read = 0;

	/* Request de upload */
	strcpy(buffer, S_UPLOAD);
	status = write(sockid, buffer, BUFFER_SIZE); // requisita upload
	if (status < 0) {
		DEBUG_PRINT("ERROR writing to socket\n");
	}

	status = read(sockid, buffer, BUFFER_SIZE); // recebe resposta
	if (status < 0) {
		DEBUG_PRINT("ERROR reading from socket\n");
	}

	if(strcmp(buffer, S_NAME) == 0) {
		getLastStringElement(buffer, file, "/"); // envia o nome do arquivo para o servidor

		status = write(sockid, buffer, BUFFER_SIZE);
		if (status < 0) {
			DEBUG_PRINT("ERROR writing to socket\n");
		}
	}

	FILE* pFile;
	pFile = fopen(file, "rb");
	if(pFile) {
		file_size = getFilesize(pFile);

		sprintf(buffer, "%d", file_size); // envia tamanho do arquivo para o servidor
		status = write(sockid, buffer, BUFFER_SIZE);
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

			// enviar buffer para salvar no servidor
			status = write(sockid, buffer, BUFFER_SIZE);
			if (status < 0) {
				DEBUG_PRINT("ERROR writing to socket\n");
			}
			//status = read(sockid, buffer, BUFFER_SIZE);
			//printf("recebido: %s", buffer);
		}

		fclose(pFile);
		status = write(sockid, buffer, BUFFER_SIZE);
		if (status < 0) {
			DEBUG_PRINT("ERROR writing to socket\n");
    }

		if(response) printf("Arquivo %s enviado.\n", file);
	} else {
		printf("Erro abrindo arquivo %s.\n", file);
	}
}

void get_file(char *file, char* fileFolder) {
	int bytes_written = 0;
	int file_size = 0;

 	/* Request de download */
	strcpy(buffer, S_DOWNLOAD);
	status = write(sockid, buffer, BUFFER_SIZE);
	if (status < 0) {
		DEBUG_PRINT("ERROR writing to socket\n");
	}

	status = read(sockid, buffer, BUFFER_SIZE); // recebe resposta
	if (status < 0) {
		DEBUG_PRINT("ERROR reading from socket\n");
	}

	if(strcmp(buffer, S_NAME) == 0) { // envia o nome do arquivo para o servidor
		DEBUG_PRINT("enviando nome do arquivo para servidor...\n");
		getLastStringElement(buffer, file, "/");

		status = write(sockid, buffer, BUFFER_SIZE);
	}
	DEBUG_PRINT("nome: %s\n", file);

	char path[MAXNAME*2 + 1];
	sprintf(path, "%s/%s", (fileFolder == NULL) ? user.folder : fileFolder, file);
	DEBUG_PRINT("path: %s\n", path);

	FILE* pFile;
	pFile = fopen(path, "wb");
	if(pFile) {
		status = read(sockid, buffer, BUFFER_SIZE); // recebe tamanho do arquivo
		if (status < 0) {
			DEBUG_PRINT("ERROR reading from socket\n");
    }

		file_size = atoi(buffer);
		DEBUG_PRINT("tamanho: %d\n", file_size);

		bytes_written = 0;
		while(file_size > bytes_written) {
			status = read(sockid, buffer, BUFFER_SIZE); // recebe arquivo no buffer
			if (status < 0) {
				DEBUG_PRINT("ERROR reading from socket\n");
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

		if(fileFolder) printf("Arquivo '%s%s%s' salvo.\n", COLOR_GREEN, path, COLOR_RESET);
	} else {
		printf("Erro salvando arquivo '%s%s%s'.\n", COLOR_GREEN, path, COLOR_RESET);
	}
}

void delete_file(char *file) {
  // avisa servidor para remover arquivo file
  /* Request de delete */
  strcpy(buffer, S_REQ_DELETE);
  status = write(sockid, buffer, BUFFER_SIZE);

  status = read(sockid, buffer, BUFFER_SIZE);
  if(strcmp(buffer, S_NAME) == 0){
		getLastStringElement(buffer, file, "/"); // envia o nome do arquivo para o servidor

  	status = write(sockid, buffer, BUFFER_SIZE);
  }

  status = read(sockid, buffer, BUFFER_SIZE);
  if(strcmp(buffer, S_RPL_DELETE) == 0){
    printf("Arquivo %s deletado!\n", file);
  }
  // recebe confirmação de que arquivo foi removido
}

void list_server() {
	int number_files = 0;

	/* Request List Server */
	strcpy(buffer, S_LS);
	status = write(sockid, buffer, BUFFER_SIZE); // requisita list server
	if (status < 0) {
		DEBUG_PRINT("ERROR writing to socket\n");
	}

	status = read(sockid, buffer, BUFFER_SIZE); // numero de arquivos no servidor
	if (status < 0) {
		DEBUG_PRINT("ERROR reading from socket\n");
	}

	number_files = atoi(buffer);
	printf("Number of files: %d\n", number_files);
	for(int i = 0; i < number_files; i++) {
		status = read(sockid, buffer, BUFFER_SIZE);
		if (status < 0) {
			DEBUG_PRINT("ERROR reading from socket\n");
		}
		printf("%s\n", buffer);
	}
	printf("Number of files: %d\n", number_files);

}

int main(int argc, char *argv[]) {
	if (argc != 4) {
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

	endereco = malloc(strlen(argv[2]));
	strcpy(endereco, argv[2]);

	porta = atoi(argv[3]);

	// Efetua conexão ao servidor
	if ((connect_server(endereco, porta))) {
		// sincroniza diretórios (cliente e servidor)
		sync_client();

		// cria a interface do cliente e espera por comandos
		show_client_interface();
	} else {
		printf("Conexão ao servidor '%s%s%s' na porta '%s%d%s' falhou.\n",
		COLOR_GREEN, endereco, COLOR_RESET, COLOR_GREEN, porta, COLOR_RESET);
	}
}
