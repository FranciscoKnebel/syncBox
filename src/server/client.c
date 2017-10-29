#include "dropboxServer.h"

char client_folder[2*MAXNAME +1];

void init_client_list(ClientList* client_list){
	client_list = NULL;
}

Client* newClient(char* userid, int socket, ClientList* client_list) {
	
	ClientList *newNode = (ClientList*)malloc(sizeof(ClientList));

	if(newNode == NULL){
        	fprintf(stderr, "Unable to allocate memory for new node\n");
        	exit(-1);
    	}
	
	Client client;
	
	strcpy(client.userid, userid);
        client.devices[0] = socket;
	client.devices[1] = -1;
	sprintf(client_folder, "%s/%s", serverInfo.folder, userid);
	client.n_files = get_dir_file_info(client_folder, client.file_info);
	client.logged_in = 1;

	newNode->client = client;
	newNode->next = NULL;
	
	if(client_list == NULL){
		client_list = newNode;
	} else{
		ClientList* current = client_list;
		while (current->next != NULL) {
    			current = current->next;
  		}
		current->next = newNode;
	}
	return &newNode->client;

}

Client* searchClient(char* userId, ClientList* client_list) {
        ClientList* current = client_list;
	while(current != NULL){
		if(strcmp(userId, current->client.userid) == 0 && current->client.logged_in == 1) {
			return &current->client;	
		}
		current = current->next;
	}
	return NULL;
}

int removeClient(Client* client, ClientList* client_list){
	ClientList* current = client_list;
	ClientList* last_visited = client_list;

	if(strcmp(client->userid, client_list->client.userid) == 0){ // testa se é o primeiro elemento
		client_list = client_list->next;
		free(current);
		return 1;
	} 
	while(current != NULL){
		if(strcmp(client->userid, current->client.userid) == 0) {
			last_visited->next = current->next;
			free(current);
			return 1;	
		}
		last_visited = current; // anterior
		current = current->next; // atual
	}
	return 0;
	
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
		return device;
	}
        check_login_status(client, client_list);

	return -1;
}

void check_login_status(Client* client, ClientList* client_list) {
	if(client->devices[0] == 0 && client->devices[1] == 0) {
		client->logged_in = 0;
		printf("Cliente %s logged out!", client->userid);
		removeClient(client, client_list);
	}
}
