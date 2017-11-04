#include "dropboxServer.h"

char client_folder[3*MAXNAME +1];

ClientList* newClient(char* userid, int socket, ClientList* client_list) {
	Client* client = (Client*) malloc(sizeof(Client));
	if(client == NULL) {
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
		DEBUG_PRINT("Inicializando fila de clientes.\n");
		client_list = (ClientList*) malloc(sizeof(ClientList));

		if(client_list == NULL) {
	  	fprintf(stderr, "Unable to allocate memory for new node\n");
	  	exit(-1);
		}
		client_list->client = client;
		client_list->next = NULL;

		return client_list;
	} else {
		DEBUG_PRINT("Adicionando cliente no final da fila.\n");
		ClientList *newNode = (ClientList*)malloc(sizeof(ClientList));

		if(newNode == NULL){
			fprintf(stderr, "Unable to allocate memory for new node\n");
			exit(-1);
		}

		newNode->client = client;
		newNode->next = NULL;

		ClientList* current = client_list;
		while (current->next != NULL) {
			current = current->next;
		}
		current->next = newNode;
		return client_list;
	}
}

Client* searchClient(char* userId, ClientList* client_list) {
  ClientList* current = client_list;

	DEBUG_PRINT("Procurando '%s' na lista de clientes.\n", userId);
	while(current != NULL){
		DEBUG_PRINT("Usuário '%s': %s.\n", current->client->userid, strcmp(userId, current->client->userid) == 0 ? "TRUE" : "FALSE");
		if(strcmp(userId, current->client->userid) == 0 && current->client->logged_in == 1) {
			return current->client;
		}
		current = current->next;
	}
	return NULL;
}

ClientList* removeClient(Client* client, ClientList* client_list){
	DEBUG_PRINT("Removendo cliente %s\n", client->userid);
	ClientList* current = client_list;
	ClientList* last_visited = client_list;

	if(strcmp(client->userid, client_list->client->userid) == 0) { // testa se é o primeiro elemento
		client_list = client_list->next;
		free(current->client);
		free(current);
		DEBUG_PRINT("Único cliente da fila removido.\n");
		return NULL;
	}

	while(current != NULL) {
		if(strcmp(client->userid, current->client->userid) == 0) {
			last_visited->next = current->next;
			free(current->client);
			free(current);
			DEBUG_PRINT("Cliente removido da lista.\n");
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
		DEBUG_PRINT("'%s' removeu dispositivo '%d'.\n", client->userid, device);
		return device;
	}

	return -1;
}

ClientList* check_login_status(Client* client, ClientList* client_list) {
	if(client->devices[0] == -1 && client->devices[1] == -1) {
		client->logged_in = 0;
		printf("Cliente '%s' logged out!\n", client->userid);

		return removeClient(client, client_list);
	}	else return client_list;
}
