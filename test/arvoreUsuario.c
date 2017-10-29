#include <stdio.h>
#include "user.h"

int testes=0,correto=0,errado=0;
extern NODE * clients;

int teste(int esperado,int resultado,const char * mensagem){
	testes++;
	char erro[100] = "";
	if(esperado == resultado)
		correto++;
	else{
		errado++;
		sprintf(erro,"erro encontrado:%d esperava:%d mensagem:%s\n",resultado,esperado,mensagem);
	}
	printf("teste %d encerrado - %s\n",testes,erro);
	return 0;
}

int relatorio(){
	printf("total:%d passados:%d errados:%d\n",testes,correto,errado );
	return 0;
}

int ambiente(){

	char f_name[] = "teste_arvore",f1_name[] = "dumb";
	char c0[] = "cliente 1",c1[] = "cliente 2",c2[] = "cliente 3";
	int resultado;

	remove_file(f_name);

	resultado = get_clients_from_file(f_name);
	teste(1,resultado,"o arquivo não deveria ser encontrado e aberto");

	resultado =  save_clients(f_name);
	teste(0,resultado,"deveria criar arquivo vazio");

	struct client * cliente = get_client(c0);
	teste(0,cliente!=NULL,"consulta encontrou cliente que não existe");

	resultado = add_client(c0);
	teste(0,resultado,"nao foi adicionado o cliente...");

	cliente = get_client(c0);
	teste(0,strcmp(cliente->userid,c0),"consulta não encontrou cliente que existe");

	resultado = add_client(c1);
	teste(0,resultado,"nao foi adicionado o cliente...");

	cliente = get_client(c1);
	teste(0,strcmp(cliente->userid,c1),"consulta não encontrou cliente que existe");

	resultado = add_client(c2);
	teste(0,resultado,"nao foi adicionado o cliente...");

	FILE *f = fopen(f1_name,"w");
	fputs("testando meus amigos",f);
	fclose(f);

	resultado = add_file_to_client(cliente,f1_name);
	teste(0,resultado,"não foi adicionado o arquivo ao usuário");

	char buffer[600];
	resultado = get_file_from_client(cliente, f1_name,buffer);
	teste(0,resultado,"arquivo não foi encontrado");
	printf("leitura do arquivo salvo - %s\n",buffer);

	resultado = add_file_to_client(cliente,f1_name);
	teste(1,resultado,"foi adicionado o mesmo arquivo ao usuário");

	resultado = remove_file_from_client(cliente, f1_name);
	teste(0,resultado,"Não conseguiu remover arquivo");

	f = fopen(f1_name,"w");
	fputs("testando novamente meus amigos",f);
	fclose(f);

	resultado = add_file_to_client(cliente,f1_name);
	teste(0,resultado,"não foi adicionado o arquivo ao usuário");

	resultado = get_file_from_client(cliente, f1_name,buffer);
	teste(0,resultado,"deveria conseguir pegar arquivo...");
	printf("leitura do arquivo salvo - %s\n",buffer);

	resultado =  save_clients(f_name);
	teste(0,resultado,"salvar clientes criados...");

	resultado = get_clients_from_file(f_name);
	teste(0,resultado,"o arquivo deveria ser encontrado e aberto");

	cliente = get_client(c1);
	teste(0,strcmp(cliente->userid,c1),"consulta não encontrou cliente que existe");

	resultado = get_file_from_client(cliente, f1_name,buffer);
	teste(0,resultado,"deveria conseguir pegar arquivo...");
	printf("leitura do arquivo salvo - %s\n",buffer);


	return 0;
}

int main(){

	puts("inicio testes");
	ambiente();
	puts("fim testes");
	relatorio();
	return 0;
}