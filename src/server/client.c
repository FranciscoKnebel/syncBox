#include "dropboxServer.h"

void clearClients() {
	for(int i = 0; i < MAX_CLIENTS; i++) {
		clients[i].logged_in = 0;
	}
}

int newClient(char* userid, int socket) {
	for(int i = 0; i < MAX_CLIENTS; i++) {
		if(!clients[i].logged_in) {
			strcpy(clients[i].userid, userid);

      clients[i].devices[0] = socket;
			clients[i].devices[1] = -1;

			char new_client_folder[2*MAXNAME +1];
	   	sprintf(new_client_folder, "%s/%s", serverInfo.folder, userid);
			clients[i].n_files = get_dir_file_info(&new_client_folder, clients[i].file_info);
			clients[i].logged_in = 1;

			return i;
		}
	}

	return MAX_CLIENTS_REACHED;
}

int searchClient(Client* client, char* userId) {
	for(int i = 0; i < MAX_CLIENTS; i++) {
		if(strcmp(userId, clients[i].userid) == 0 && clients[i].logged_in == 1) {
			client = &clients[i];
			return i;
		}
	}

	return CLIENT_NOTFOUND;
}

int addDevice(Client* client, int socket) {
	if(client->devices[0] == -1) {
		client->devices[0] = socket;
		return 0;
  }

  if(client->devices[1] == -1) {
		client->devices[1] = socket;
		return 1;
  }

	return -1;
}

int removeDevice(Client* client, int device) {
	if(client) {
		client->devices[device] = -1;
		return device;
	}

	return -1;
}

void check_login_status(Client* client) {
	if(client->devices[0] == 0 && client->devices[1] == 0) {
		client->logged_in = 0;
		printf("Cliente %s logged out!", client->userid);
	}
}
