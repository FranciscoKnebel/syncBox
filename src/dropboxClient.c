#include "dropboxClient.h"

struct user_info user;

int connect_server (char *host, int port) {
	// se não conectou
	// return 0

	return 1;
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

	// else {
		// sincroniza pasta local com o servidor
		// usar inotify ou dirent

		// se um arquivo local foi criado, adicionar ao servidor
		// send_file

		// se um arquivo local foi apagado, remover do servidor
		// delete_file

		// se há um conflito entre cópias do mesmo arquivo entre cliente e servidor,
		// apagar arquivo local e trazer do servidor.
		// delete_file, mas apenas local.
		// get_file
	// }
}

void send_file(char *file) {
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

		while(!feof(pFile)) {
				fread(buffer, sizeof(char), BUFFER_SIZE, pFile);
				bytes_read += sizeof(char) * BUFFER_SIZE;

				// enviar buffer para salvar no servidor
		}

		fclose(pFile);
		printf("Arquivo %s enviado.\n", file);
	} else {
		printf("Erro abrindo arquivo %s.\n", file);
	}

}

void get_file(char *file) {
	int bytes_written = 0;

	FILE* pFile;
	char buffer[BUFFER_SIZE]; // 1 KB buffer

	pFile = fopen(file, "wb");
	if(pFile) {
		// requisita arquivo file para o servidor
		// recebe buffer do servidor

		// while( faltam bytes para escrever ) {
			fwrite(buffer, sizeof(char), BUFFER_SIZE, pFile);
			bytes_written += sizeof(char) * BUFFER_SIZE;
		// }
		fclose(pFile);

		printf("Arquivo %s salvo.\n", file);
	} else {
		printf("Erro abrindo arquivo %s.\n", file);
	}
}

void delete_file(char *file) {
	// avisa servidor para remover arquivo file
	// recebe confirmação de que arquivo foi removido
}

void close_connection() {

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

		char username[MAXNAME];
		strcpy(username, getUserName());

		sprintf(user.folder, "/home/%s/sync_dir_%s", username, user.id);
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
		puts("Conexão ao servidor falhou.");
	}
}
