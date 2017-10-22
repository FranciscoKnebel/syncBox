#include "user.h"
#include "fileUtil.c"

NODE *clients = NULL;

int comp_clients(void *c1,void *c2){

	return strcmp(((Client*)c1)->userid,((Client*)c2)->userid);
}

int is_client(void *c1,void *c2){

	return !strcmp(((Client*)c1)->userid,(char*)c2);
}


int * get_clients_from_file(const char *file_name){
	
	if(!file_name)
		return 1;

	FILE *f = fopen(file_name,"r");

	if(!f)
		return 1;

	clients = mount_tree(f,comp_clients,sizeof(Client));

	return 0;
}

int add_client(const char *client_name){
	if(!client_name)
		return 1;

	Client * new_c = malloc(sizeof(Client));
	strncpy(new_c->userid,client_name, (MAXNAME < strlen(client_name)) ? MAXNAME : strlen(client_name));
	new_c->n_files = 0;
	clients = insert_node(clients,(void*)new_c,comp_clients);
	return 0;
}

Client *get_client(const char *client_name){

	if(!client_name)
		return NULL;
	void *res_search = get_value(clients,(void*)client_name,is_client);
	if(res_search == NULL)
		return NULL;
	else
		return (Client*)res_search;

}

int add_file_to_client(Client *c,const char* file_name){

	if(!c || !file_name)
		return 1;

	FILE *f = fopen(file_name);
	fseek(f, 0, SEEK_END); // seek to end of file
	int size = ftell(f); // get current file pointer
	fclose(f);

	FileInfo *info = malloc(sizeof(FileInfo));
	strcpy(info->name,file_name);

	char *extention;
	get_file_extension(file_name,&extention);
	strcpy(info->extension,extention);

	char *mod;
	get_last_modification_time(file_name,&mod);
	strcpy(info->last_modified,mod);
	
	info->size = size;

	c->file_info[c->n_files] = info;
	c->n_files++;

	free(extention);
	free(mod);

	return 0;
}

int remove_file_from_client(Client *c,const char* file_name){

	if(!c || !file_name)
		return 1;

	int i=0;
	for(;i<c->n_files && strcmp(c->file_info[i]->name,file_name);i++);
	if(i == c->n_files)
		return 1;
	while(i != c->n_files && (i+1 != MAXFILES)){
		c->file_info[i] = c->file_info[i+1];
	}
	return 0;
}
int get_file_from_client(Client *c,const char* file_name,char * buffer){

	int i=0;

	if(!c || !file_name || !buffer)
		return 1;

	for(;i<c->n_files && strcmp(c->file_info[i]->name,file_name);i++);
	if(i == c->n_files)
		return 1;

	char *temp_buffer;
	read_file(&temp_buffer,file_name);
	strcpy(buffer,temp_buffer);
	free(temp_buffer);
	return 0;
}


int main(){

	
}