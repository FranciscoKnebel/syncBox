#include "dropboxClient.h"

char userid[MAXNAME];

int connect_server (char *host, int port) {
	return 1;
}

void sync_client() {

}

void send_file(char *file) {

}

void get_file(char *file) {

}

void delete_file(char *file) {

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
		strcpy(userid, argv[1]);
	} else {
		puts("Tamanho máximo de userid foi ultrapassado.");
		printf("Máximo: %d. Inserido: %d.\n", MAXNAME, strlen(argv[1]) <= MAXNAME);
		return MAXNAMESIZE_REACHED;
	}

	endereco = malloc(sizeof(argv[2]));
	strcpy(endereco, argv[2]);

	porta = atoi(argv[3]);

	//tenta conectar ao servidor
	if ((connect_server(endereco, porta)) > 0) {
		//sincroniza diretórios (cliente e servidor)
		sync_client();

		//cria a interface do cliente e espera por comandos
		show_client_interface();
	}
}
