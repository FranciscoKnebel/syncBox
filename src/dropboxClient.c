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

	bzero(buffer, BUFFER_SIZE);
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
		printf("ERROR writing to socket\n");
		return 1;
	}

	status = read(sockid, buffer, BUFFER_SIZE);
	if (status < 0) {
		printf("ERROR reading from socket\n");
		exit(1);
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

	/* Request de upload */
	strcpy(buffer, S_UPLOAD);
	status = write(sockid, buffer, BUFFER_SIZE); // requisita upload
	// TODO teste de status

	status = read(sockid, buffer, BUFFER_SIZE); // recebe resposta
	// TODO teste de status

	if(strcmp(buffer, S_NAME) == 0) {
		strcpy(buffer, file); // envia o nome do arquivo para o servidor
		status = write(sockid, buffer, BUFFER_SIZE);
		// TODO teste de status
	}

	FILE* pFile;
	pFile = fopen(file, "rb");
	if(pFile) {
		file_size = getFilesize(pFile);

		sprintf(buffer, "%d", file_size); // envia tamanho do arquivo para o servidor
		status = write(sockid, buffer, BUFFER_SIZE);
		// TODO teste de status

		if(file_size == 0) {
			fclose(pFile);
			return 1;
		}

		while(!feof(pFile)) {
			fread(buffer, sizeof(char), BUFFER_SIZE, pFile);
			bytes_read += sizeof(char) * BUFFER_SIZE;

			// enviar buffer para salvar no servidor
			status = write(sockid, buffer, BUFFER_SIZE);
			// TODO teste de status
			//status = read(sockid, buffer, BUFFER_SIZE);
			//printf("recebido: %s", buffer);
		}

		fclose(pFile);
		status = write(sockid, buffer, BUFFER_SIZE);
		// TODO teste de status

		printf("Arquivo %s enviado.\n", file);
		return status;
	} else {
		printf("Erro abrindo arquivo %s.\n", file);
	}
}

void get_file(char *file) {
	int bytes_to_read = 0;
	int bytes_written = 0;
	int file_size = 0;

 	/* Request de download */
	strcpy(buffer, S_DOWNLOAD);
	status = write(sockid, buffer, BUFFER_SIZE);
	// TODO teste de status

	status = read(sockid, buffer, BUFFER_SIZE); // recebe resposta
	// TODO teste de status

	if(strcmp(buffer, S_NAME) == 0) { // envia o nome do arquivo para o servidor
		DEBUG_PRINT("envia...\n");
		strcpy(buffer, file);
		status = write(sockid, buffer, BUFFER_SIZE);
	}
	DEBUG_PRINT("nome: %s\n", file);

	char path[MAXNAME*2];
	sprintf(path, "%s/%s", user.folder, file);
	DEBUG_PRINT("%s\n", path);

	FILE* pFile;
	 // 1 KB buffer
	pFile = fopen(path, "wb");
	if(pFile) {
		status = read(sockid, buffer, BUFFER_SIZE); // recebe tamanho do arquivo
		// TODO teste de status

		file_size = atoi(buffer);
		DEBUG_PRINT("tamanho: %d\n", file_size);

		bytes_written = 0;
		bytes_to_read = file_size;
		while(file_size > bytes_written) {
			status = read(sockid, buffer, BUFFER_SIZE); // recebe arquivo no buffer
			// TODO teste de status

			if(bytes_to_read > BUFFER_SIZE) { // se o tamanho do arquivo for maior, lê buffer completo
				fwrite(buffer, sizeof(char), BUFFER_SIZE, pFile);
				bytes_written += sizeof(char) * BUFFER_SIZE;
				bytes_to_read -= bytes_to_read;
			} else { // senão lê só o bytes_to_read
				fwrite(buffer, sizeof(char), bytes_to_read, pFile);
				bytes_written += sizeof(char) * bytes_to_read;
			}

			DEBUG_PRINT("leu\n");
		}

		fclose(pFile);

		printf("Arquivo %s salvo.\n", path);
	} else {
		printf("Erro abrindo arquivo %s.\n", path);
	}
}

void delete_file(char *file) {
	// avisa servidor para remover arquivo file
	// recebe confirmação de que arquivo foi removido
}

void list_server() {
	int number_files = 0;

	/* Request List Server */
	strcpy(buffer, S_LS);
	status = write(sockid, buffer, BUFFER_SIZE); // requisita list server
	// TODO teste de status

	status = read(sockid, buffer, BUFFER_SIZE); // numero de arquivos no servidor
	// TODO teste de status

	number_files = atoi(buffer);
	printf("%d\n", number_files);
	for(int i = 0; i < number_files; i++) {
		status = read(sockid, buffer, BUFFER_SIZE);
		// TODO teste de status
		printf("%s\n", buffer);
	}
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
		printf("Conexão ao servidor '%s%s%s' na porta '%s%d%s' falhou.\n",
		COLOR_GREEN, endereco, COLOR_RESET, COLOR_GREEN, porta, COLOR_RESET);
	}
}
