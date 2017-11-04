#include "dropboxServer.h"

char client_folder[3*MAXNAME +1];


ClientList* newClient(char* userid, int socket, ClientList* client_list) {

	Client* client = (Client*) malloc(sizeof(Client));
	if(client == NULL){
		fprintf(stderr, "Unable to allocate memory for new node\n");
		exit(-1);
	}
	strcpy(client->userid, userid);
	client->devices[0] = socket;
	client->devices[1] = -1;
	sprintf(client_folder, "%s/%s", serverInfo.folder, userid);
	client->n_files = get_dir_file_info(client_folder, client->file_info);
	client->logged_in = 1;



	if(client_list == NULL){
		client_list = (ClientList*) malloc(sizeof(ClientList));
		if(client_list == NULL){
	  	fprintf(stderr, "Unable to allocate memory for new node\n");
	  	exit(-1);
		}
		client_list->client = client;
		client_list->next = NULL;
		DEBUG_PRINT("Clientlist vazia. inserindo no primeiro\n");
		DEBUG_PRINT("%s\n", client_list->client->userid);
		return client_list;
	} else{
			ClientList *newNode = (ClientList*)malloc(sizeof(ClientList));
			if(newNode == NULL){
  			fprintf(stderr, "Unable to allocate memory for new node\n");
  			exit(-1);
			}

			newNode->client = client;
			newNode->next = NULL;

			int i = 0;
			ClientList* current = client_list;
			while (current->next != NULL) {
				i++;
				current = current->next;
			}
			current->next = newNode;
			DEBUG_PRINT("Clientlist nao vazia. Inserindo no %d\n", i);
			return client_list;
		}


}

Client* searchClient(char* userId, ClientList* client_list) {
	DEBUG_PRINT("Iniciando searchClient!\n");
	if(client_list){
		DEBUG_PRINT("ClientList existe!!\n");
		DEBUG_PRINT("client list existe, nome do primeiro elemento: %s\n", client_list->client->userid);
		if(!client_list->next){
			DEBUG_PRINT("search client: next nulo\n");
		}
	}
	else{
		DEBUG_PRINT("ClientList nula!\n");
	}

  ClientList* current = client_list;
	int i = 0;
	while(current != NULL){
		i++;
		if(strcmp(userId, current->client->userid) == 0 && current->client->logged_in == 1) {
			DEBUG_PRINT("Cliente encontrado!\n");
			return current->client;
		}
		current = current->next;
		DEBUG_PRINT("next... %d\n", i);
	}
	DEBUG_PRINT("Cliente nao encontrado!\n");
	return NULL;
}

ClientList* removeClient(Client* client, ClientList* client_list){
	DEBUG_PRINT("Removendo cliente\n");
	ClientList* current = client_list;
	ClientList* last_visited = client_list;

	if(strcmp(client->userid, client_list->client->userid) == 0) { // testa se é o primeiro elemento
		client_list = client_list->next;
		free(current->client);
		free(current);
		DEBUG_PRINT("Unico Cliente removido!\n");
		return NULL;
	}

	while(current != NULL) {
		if(strcmp(client->userid, current->client->userid) == 0) {
			last_visited->next = current->next;
			free(current->client);
			free(current);
			DEBUG_PRINT("Cliente removido não unico!\n");
		}
		last_visited = current; // anterior
		current = current->next; // atual
	}
	return client_list;
}

int addDevice(Client* client, int socket) {
  if(client->devices[0] == -1) {
   	sprintf(client_folder, "%s/%s", serverInfo.folder, client->userid);
    client->n_files = get_dir_file_info(client_folder, client->file_info);
		client->devices[0] = socket;

		return 0;
  }

  if(client->devices[1] == -1) {
   	sprintf(client_folder, "%s/%s", serverInfo.folder, client->userid);
    client->n_files = get_dir_file_info(client_folder, client->file_info);
		client->devices[1] = socket;
		return 1;
  }

  return -1;
}

int removeDevice(Client* client, int device, ClientList* client_list) {
	if(client) {
		client->devices[device] = -1;
		DEBUG_PRINT("Removeu dispositivo!\n");
		return device;
	}

	return -1;
}

ClientList* check_login_status(Client* client, ClientList* client_list) {
	if(client->devices[0] == -1 && client->devices[1] == -1) {
		client->logged_in = 0;
		printf("Cliente %s logged out!\n", client->userid);
		return removeClient(client, client_list);
	}
	else return client_list;
}
