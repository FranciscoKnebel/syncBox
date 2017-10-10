#include "../headers/dropboxUtil.h"

char userid[MAXNAME];

void main(int argc, char *argv[]){

	if (argc < 3){
		printf("Argumentos Insuficientes.\n");
		exit(0);
	}

	int porta;
	char *endereco;


	//user
	if (strlen(argv[1]) <= MAXNAME)
		strcpy(userid, argv[1]);

	//endereço
	endereco = malloc(sizeof(argv[2]));
	strcpy(endereco, argv[2]);

	//porta
	porta = atoi(argv[3]);


	//tenta conectar ao servidor
	if ((connect_server(endereco, porta)) > 0)
	{
		//sincroniza diretórios (cliente e servidor)
		sync_client();

		//cria a interface do cliente e espera por comandos
		show_client_interface();
	}
}

int connect_server (char *host, int port){
	return 1;
}

void sync_client(){

}

void send_file(char *file){

}

void get_file(char *file){

}

void delete_file(char *file){

}
void close_connection(){

}

void show_client_interface()
{
	char comando_solicitado[100], nome_arquivo[100];

	//TODO ajustar o resto dos comandos
	printf("\nComandos do cliente:\nupload <path/filename.ext>\ndownload <filename.ext>\n[resto dos comandos...]\nsair\n");

	while(strcmp(comando_solicitado, "sair\n") != 0){ // strcmp retém o ENTER, necessário "\n"
		printf("\nDigite seu comando: ");

		fgets(comando_solicitado, sizeof(comando_solicitado), stdin);
	}

}
