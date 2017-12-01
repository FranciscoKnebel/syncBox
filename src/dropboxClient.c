#include "dropboxClient.h"

char buffer[BUFFER_SIZE];
pthread_t sync_thread;
struct user_info user;
int sockid;
int status;

int connect_server (char *host, int port) {
	struct sockaddr_in serverconn;
	/* Create a socket point */
	sockid = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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
		return 0;
	}

	bzero(buffer, BUFFER_SIZE);
	strcpy(buffer, user.id);

	// write to socket
	write_to_socket(sockid, buffer);

	// read server response
	bzero(buffer, BUFFER_SIZE);
	read_from_socket(sockid, buffer);

	if(strcmp(buffer, S_EXCESS_DEVICES) == 0) {
		printf("Muitas conexões simultâneas do mesmo usuário.\n");
	}

	if(strcmp(buffer, S_CONNECTED) == 0) {
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
	write_to_socket(sockid, buffer);

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
	synchronize_local(sockid);

	// sincroniza servidor com pasta local
	synchronize_server(sockid);

	// cria thread para manter a sincronização local
	int rc;
	if((rc = pthread_create(&sync_thread, NULL, watcher_thread, (void *) user.folder))) {
		printf("Syncronization Thread creation failed: %d\n", rc);
	}
}

void send_file(char *file, int response) {
	int file_size = 0;
	int bytes_sent = 0;

	DEBUG_PRINT("Requisita upload\n");
	/* Request de upload */
	strcpy(buffer, S_UPLOAD);
	write_to_socket(sockid, buffer); // requisita upload

	read_from_socket(sockid, buffer); // recebe resposta "name"

	if(strcmp(buffer, S_NAME) == 0) {
		getLastStringElement(buffer, file, "/"); // envia o nome do arquivo para o servidor
		DEBUG_PRINT("Nome enviado: %s\n", buffer);
		write_to_socket(sockid, buffer);
	}

	read_from_socket(sockid, buffer); // le palavra "timestamp"
	DEBUG_PRINT("recebido: %s\n", buffer);

	if(strcmp(buffer, S_MODTIME) == 0) { // se palavra for igual a "timestamp"
		getFileModifiedTime(file, buffer);
		DEBUG_PRINT("MT enviado: %s\n", buffer);
		write_to_socket(sockid, buffer); // envia timestamp
	}

	FILE* pFile;
	pFile = fopen(file, "rb");
	if(pFile) {
		file_size = getFilesize(pFile);

		sprintf(buffer, "%d", file_size); // envia tamanho do arquivo para o servidor
		write_to_socket(sockid, buffer);

		if(file_size == 0) {
			fclose(pFile);
		} else {
			DEBUG_PRINT("Começando a enviar arquivo de %d bytes.\n", file_size);
      bytes_sent = 0;
      while(bytes_sent < file_size) {
	      if((file_size - bytes_sent) > BUFFER_SIZE) {
	        fread(buffer, sizeof(char), BUFFER_SIZE, pFile);
	        bytes_sent += sizeof(char) * BUFFER_SIZE;
	      } else { // senão lê os bytes que sobram
	        fread(buffer, sizeof(char), (file_size - bytes_sent), pFile);
	        bytes_sent += sizeof(char) * (file_size - bytes_sent);
	      }

				// enviar buffer para salvar no servidor
				write_to_socket(sockid, buffer);
			}
			DEBUG_PRINT("Terminou de enviar arquivo.\n");
			fclose(pFile);
		}

		if(response) {
			printf("Arquivo %s enviado.\n", file);
		}
	} else {
		printf("Erro abrindo arquivo %s.\n", file);
		strcpy(buffer, S_ERRO_ARQUIVO);
		write_to_socket(sockid, buffer);
	}

	bzero(buffer, BUFFER_SIZE);
}

void get_file(char *file, char* fileFolder) {
	int bytes_written = 0;
	int file_size = 0;

 	/* Request de download */
	strcpy(buffer, S_DOWNLOAD);
	write_to_socket(sockid, buffer); // envia "download"

	read_from_socket(sockid, buffer); // recebe resposta "name"
	DEBUG_PRINT("Resposta recebida: %s \n", buffer);

	if(strcmp(buffer, S_NAME) == 0) { // envia o nome do arquivo para o servidor
		DEBUG_PRINT("enviando nome do arquivo para servidor\t\n");
		getLastStringElement(buffer, file, "/");

		write_to_socket(sockid, buffer);
	}
	DEBUG_PRINT("nome: %s\n", buffer);

	char path[MAXNAME*2 + 1];
	sprintf(path, "%s/%s", (fileFolder == NULL) ? user.folder : fileFolder, file);
	DEBUG_PRINT("path: %s\n", path);

	read_from_socket(sockid, buffer); // recebe tamanho do arquivo ou mensagem de erro

	if(strcmp(buffer, S_ERRO_ARQUIVO) != 0) {
		FILE* pFile;

		pFile = fopen(path, "wb");
		if(pFile) {
			file_size = atoi(buffer);
			DEBUG_PRINT("tamanho: %d\n", file_size);

			read_from_socket(sockid, buffer); // recebe modified time do arquivo

			time_t modified_time = getTime(buffer);
			DEBUG_PRINT("MT: %s\n", buffer);

			if(file_size == 0) { // se tamanho for 0
				read_from_socket(sockid, buffer); // recebe arquivo no buffer
			}

			bytes_written = readToFile(pFile, file_size, sockid);
			if(bytes_written == file_size) {
				DEBUG_PRINT("Terminou de escrever.\n");
			} else {
				DEBUG_PRINT("Erro ao escrever %d bytes. Esperado %d.\n", bytes_written, file_size);
			}

			fclose(pFile);
			setModTime(path, modified_time);

			if(fileFolder) printf("Arquivo '%s%s%s' salvo.\n", COLOR_GREEN, path, COLOR_RESET);
		} else {
			printf("Erro salvando arquivo '%s%s%s'.\n", COLOR_GREEN, path, COLOR_RESET);
		}
	} else{
		printf("Erro ao receber arquivo do servidor '%s%s%s'.\n", COLOR_GREEN, path, COLOR_RESET);
	}

	bzero(buffer, BUFFER_SIZE);
}

void delete_file(char *file) {
  // avisa servidor para remover arquivo file
  /* Request de delete */
  char filename[MAXNAME];
  strcpy(buffer, S_REQ_DELETE);
  write_to_socket(sockid, buffer);
  read_from_socket(sockid, buffer); // recebe name

  if(strcmp(buffer, S_NAME) == 0) {
		getLastStringElement(buffer, file, "/"); // envia o nome do arquivo para o servidor
		sprintf(filename, "%s", buffer);
  	write_to_socket(sockid, buffer); // envia nome do arquivo
  }

  read_from_socket(sockid, buffer);

  if(strcmp(buffer, S_RPL_DELETE) == 0){
		// recebe confirmação de que arquivo foi removido
    DEBUG_PRINT("Arquivo %s deletado!\n", filename);
  }

	bzero(buffer, BUFFER_SIZE);
}

void list_server() {
	int number_files = 0;

	/* Request List Server */
	strcpy(buffer, S_LS);
	write_to_socket(sockid, buffer); // requisita list server

	status = read(sockid, buffer, BUFFER_SIZE); // numero de arquivos no servidor
	if (status < 0) {
		DEBUG_PRINT("ERROR reading from socket\n");
	}

	number_files = atoi(buffer);
	printf("Number of files: %d\n", number_files);
	for(int i = 0; i < number_files; i++) {
		read_from_socket(sockid, buffer);
		printf("%s\n", buffer);
	}
	printf("Number of files: %d\n", number_files);

	bzero(buffer, BUFFER_SIZE);
}

int main(int argc, char *argv[]) {
	setlocale(LC_ALL, "pt_BR");

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
