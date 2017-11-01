#ifndef USER_C
#define USER_C

#include "dropboxServer.h"
#include <semaphore.h> 
 
sem_t mutex; 
sem_t acess_tree; 
 
int leitores = 0; 
 
void startSem(){ 
  //inicializa os semáforos de modo apropriado 
  sem_init(&mutex, 0, 1); 
  sem_init(&acess_tree, 0, 1); 
} 

NODE *clients = NULL;
int total_clients = 0;

int comp_clients(void *c1,void *c2){
/*
função para definir as posições dos clientes na árvore, utiliza o id!
*/

	return strcmp(((Client*)c1)->userid,((Client*)c2)->userid);
}

int is_client(void *c1,void *c2){
/*
função para encontrar alvo, deve retornar verdade(!= 0) quando
encontrar o alvo! 
*/

	return 0==strcmp(((Client*)c1)->userid,(char*)c2);
}


int get_clients_from_file( char *file_name){
/*
recupera os clientes de um arquivo!
*/
	
	if(!file_name)
		return 1;

	FILE *f = fopen(file_name,"rb");

	if(!f)
		return 1;

	fseek(f, 0, SEEK_END); // seek to end of file
	int size = ftell(f);
	rewind(f);
	if(size < sizeof(Client))
		return 1;

	fscanf(f,"%d\n",&total_clients);
	
	sem_wait(&acess_tree);

	clients = mount_tree(f,comp_clients,sizeof(Client),total_clients);

	sem_post(&acess_tree);

	return 0;
}

int add_client( char *client_name){
/*
adiciona novo cliente na árvore global de clientes do servidor,
deve ser observado que a posição depende da função definida bem acima!
novos clientes tem 0 arquivos por definição!
*/
	if(!client_name)
		return 1;

	Client * new_c = malloc(sizeof(Client));

	strncpy(new_c->userid,client_name, MAXNAME);

	new_c->n_files = 0;
	int i=0;
	for(i=0;i<2;i++)
		new_c->devices[i] = 0; //0 é disponível

	sem_wait(&acess_tree);
	clients = insert_node(clients,(void*)new_c,comp_clients);
	total_clients++;
	sem_post(&acess_tree);

	return 0;
}

Client *get_client( char *client_name){

/*
busca o cliente na árvore de clientes, utiliza a função definida bem acima para encontrar!
*/

	if(!client_name)
		return NULL;
	sem_wait(&mutex);
	leitores++;
	if(leitores == 1){
		sem_wait(&acess_tree);
	}
	sem_post(&mutex);

	void *res_search = get_value(clients,(void*)client_name,is_client);
	
	sem_wait(&mutex);
	leitores--;
	if(leitores == 0){
		sem_post(&acess_tree);
	}
	sem_post(&mutex);

	if(res_search == NULL)
		return NULL;
	else
		return (Client*)res_search;

}

int add_file_to_client(Client *c, char* file_name){
/*
adição de arquivo no cliente. O arquivo deve existir, logo
esse controle da posição do arquivo não faz parte desta função,
aqui basicamente ocorre a ligação entre arquivo e cliente!
*/
	if(!c || !file_name)
		return 1;


	/*buscamos o tamanho do arquivo!*/
	FILE *f = fopen(file_name,"r");
	if(f == NULL)
		return 1;
	fseek(f, 0, SEEK_END); // seek to end of file
	int size = ftell(f); // get current file pointer
	fclose(f);

	int i;
	for(;i<c->n_files && strcmp(c->file_info[i].name,file_name);i++);
	if(i != c->n_files)
		return 1;

	FileInfo *info = malloc(sizeof(FileInfo));
	strcpy(info->name,file_name);

	char *extention;
	get_file_extension(file_name,&extention);
	strcpy(info->extension,extention);

	char *mod;
	get_last_modification_time(file_name,&mod);
	strcpy(info->last_modified,mod);
	
	info->size = size;

	/*aumentamos o número de arquivos do usuário*/
	c->file_info[c->n_files] = *info;
	c->n_files++;
	free(info);

	free(extention);
	free(mod);

	return 0;
}

int remove_file_from_client(Client *c, char* file_name){
/*
remoção da conexão do usuário com o arquivo alvo
*/
	if(!c || !file_name)
		return 1;

	/*for para encontrar o arquivo i no usuário e
	 fazer com que o valor de i seja equivalente a sua posição*/
	int i=0;
	for(;i<c->n_files && strcmp(c->file_info[i].name,file_name);i++);
	if(i == c->n_files)
		return 1;
	/*sobre-escreve o arquivo a ser deletado com os outros
	para evitar buracos no array
	básicamente um SHIFT com perda*/
	while(i != c->n_files && (i+1 != MAXFILES)){
		c->file_info[i] = c->file_info[i+1];
		i++;
	}
	c->n_files--;
	return 0;
}
int get_file_from_client(Client *c, char* file_name,char * buffer){
/*
pegar conteudo de arquivo do usuário
*/
	int i=0;

	if(!c || !file_name || !buffer)
		return 1;

	/*utiliza o for para encontrar o arquivo que se deseja ler, posição em i*/
	for(;i<c->n_files && strcmp(c->file_info[i].name,file_name);i++);
	if(i == c->n_files)
		return 1;

	FILE *f = fopen(file_name,"r");
	int size;
	fseek(f, 0, SEEK_END);
	size = ftell(f);
	rewind(f);
	fclose(f);

	read_file(buffer,size,file_name);

	return 0;
}

int save_clients(char file_name[]){

	if(file_name == NULL)
		return 1;

	FILE * f = fopen(file_name,"wb");

	fprintf(f,"%d\n",total_clients);

	if(f == NULL)
		return 2;

	sem_wait(&mutex);
	leitores++;
	if(leitores == 1){
		sem_wait(&acess_tree);
	}
	sem_post(&mutex);

	int exec =  save_tree(f,clients,sizeof(Client));

	sem_wait(&mutex);
	leitores--;
	if(leitores == 0){
		sem_post(&acess_tree);
	}
	sem_post(&mutex);

	fclose(f);
		
	return exec;
}
#endif
