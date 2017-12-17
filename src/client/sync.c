#include "dropboxClient.h"

void synchronize_local(SSL *ssl, int print) { // executa primeiro
  FileInfo localFiles[MAXFILES];
  int number_files_client = 0;
  char path[MAXNAME * 3 + 1];
  char buffer[BUFFER_SIZE];
  char file_name[MAXNAME];
  char last_modified[MAXNAME];
  int number_files_server = 0;

  number_files_client = get_dir_file_info(user.folder, localFiles); // pega os arquivos do cliente

  int files_to_delete[number_files_client];
  for(int i = 0; i < number_files_client; i++){
    files_to_delete[i] = 0;
  }

  DEBUG_PRINT_COND(print, "Iniciando sincronização local.\n");

  strcpy(buffer, S_SYNC);
	write_to_socket(ssl, buffer); // envia comando "sync"

	read_from_socket(ssl, buffer); // recebe numero de arquivos no server

	number_files_server = atoi(buffer);
  DEBUG_PRINT_COND(print, "'%s%d%s' arquivos no servidor\n", COLOR_GREEN, number_files_server, COLOR_RESET);

	char last_modified_2[MAXNAME];
	for(int i = 0; i < number_files_server; i++) {
		read_from_socket(ssl, buffer); // nome do arquivo no server
		strcpy(file_name, buffer);
    DEBUG_PRINT_COND(print, "\n");
    DEBUG_PRINT_COND(print, "%d: Nome recebido: %s\n", i, file_name);

    for(int i = 0; i < number_files_client; i++) {
      if(strcmp(file_name, localFiles[i].name) == 0){
        files_to_delete[i] = 1;
      }
    }

		read_from_socket(ssl, buffer); // timestamp
		strcpy(last_modified, buffer);
    DEBUG_PRINT_COND(print, "%d: Last modified recebido: %s\n", i, last_modified);

		sprintf(path, "%s/%s", user.folder, file_name);
    DEBUG_PRINT_COND(print, "%d: Path: %s\n", i, path);

		if(!fileExists(path)) {
      DEBUG_PRINT_COND(print, "%d: Arquivo %s não existe... baixando\n", i, file_name);
	    get_file(file_name, NULL);
		} else {
      DEBUG_PRINT_COND(print, "%d: Arquivo existe, verificando se necessita baixar...\n",  i);
      getFileModifiedTime(path, last_modified_2);
      DEBUG_PRINT_COND(print, "%d: Last modified local: %s\n", i, last_modified_2);

      if (older_file(last_modified, last_modified_2) == 1) {
        DEBUG_PRINT_COND(print, "%d: Arquivo %s mais velho... baixando\n", i, file_name);
        get_file(file_name, NULL);
      } else {
        DEBUG_PRINT_COND(print, "%d: Download desnecessário do arquivo %s.\n", i, file_name);
  			strcpy(buffer, S_OK);
  			write_to_socket(ssl, buffer);
  		}
    }
	}

  for(int i = 0; i < number_files_client; i++){
    if(files_to_delete[i] == 0){
      sprintf(path, "%s/%s", user.folder, localFiles[i].name);
      if(remove(path) == 0) {
        DEBUG_PRINT_COND(print, "Arquivo %s deletado!\n", path);
      }
    }
  }
  DEBUG_PRINT_COND(print, "Encerrando sincronização local.\n\n");
}

void synchronize_server(SSL *ssl) {
  FileInfo localFiles[MAXFILES];
  char path[MAXNAME * 3 + 1];
  char buffer[BUFFER_SIZE];
  int number_files_client = 0;

  DEBUG_PRINT("Iniciando sincronização do servidor.\n");

  number_files_client = get_dir_file_info(user.folder, localFiles);
	sprintf(buffer, "%d", number_files_client);
	write_to_socket(ssl, buffer); // envia numero de arquivos do cliente pro server

	for(int i = 0; i < number_files_client; i++) {
		strcpy(buffer, localFiles[i].name);
    DEBUG_PRINT("\n");
    DEBUG_PRINT("%d: Nome enviado: %s\n", i, localFiles[i].name);
		write_to_socket(ssl, buffer);
		strcpy(buffer, localFiles[i].last_modified);
    DEBUG_PRINT("%d: Last modified enviado: %s\n", i, localFiles[i].last_modified);
		write_to_socket(ssl, buffer);

		read_from_socket(ssl, buffer); // resposta do server

    DEBUG_PRINT("%d: Recebido: %s\n", i, buffer);
		if(strcmp(buffer, S_GET) == 0) {
			sprintf(path, "%s/%s", user.folder, localFiles[i].name);
			send_file(path, FALSE);
      DEBUG_PRINT("%d: Enviando arquivo %s\n", i, path);
		} else if (strcmp(buffer, S_OK) == 0) {
      DEBUG_PRINT("%d: Envio desnecessário.\n", i);
    }
	}

  DEBUG_PRINT("Encerrando sincronização do servidor.\n");
}

void *watcher_thread(void* ptr_path) {
  char* watch_path = malloc(strlen((char*) ptr_path));
  strcpy(watch_path, (char*) ptr_path);

  int fd, wd;
  int length, i = 0;
  char buffer[WATCHER_EVENT_BUF_LEN];

  fd = inotify_init();
  if(fd < 0) {
    perror("inotify_init");
  }

  wd = inotify_add_watch(fd, watch_path, IN_CLOSE_WRITE | IN_CREATE | IN_DELETE | IN_DELETE_SELF | IN_MOVED_FROM | IN_MOVED_TO);

  char path[MAXNAME];
  int thread_running = TRUE;
  while(thread_running) {
    length = read(fd, buffer, WATCHER_EVENT_BUF_LEN);

    if (length < 0) {
      thread_running = FALSE;
    } else {
      i = 0;
      while (i < length) {
        struct inotify_event* event = (struct inotify_event *) &buffer[i];

        if (event->len) {
          sprintf(path, "%s/%s", watch_path, event->name);

          int not_a_temp_file = (event->name[0] != '.') && (event->name[strlen(event->name) - 1] != '~');

          if (event->mask & (IN_CLOSE_WRITE | IN_CREATE | IN_MOVED_TO)) {
            if (fileExists(path) && not_a_temp_file) {
              DEBUG_PRINT("Request upload: %s\n", path);
              pthread_mutex_lock(&mutex_watcher);

              strcpy(buffer, ".");
              write_to_socket(ssl, buffer);

              send_file(path, FALSE);

              pthread_mutex_unlock(&mutex_watcher);
              DEBUG_PRINT("Enviou!\n");
            }
          } else if (event->mask & (IN_DELETE | IN_DELETE_SELF | IN_MOVED_FROM)) {
            if (not_a_temp_file) {
              DEBUG_PRINT("Request delete: %s\n", path);
              pthread_mutex_lock(&mutex_watcher);

              strcpy(buffer, ".");
              write_to_socket(ssl, buffer);

              delete_file(path);

              pthread_mutex_unlock(&mutex_watcher);
              DEBUG_PRINT("Deletou!\n");
            }
          }
        }

        i += WATCHER_EVENT_SIZE + event->len;
      }
    }

    usleep(1000);
  }

  inotify_rm_watch(fd, wd);
  close(fd);

  return 0;
}

void* sync_devices_thread() {
  char buffer[BUFFER_SIZE];

	DEBUG_PRINT("%sSYNC: Thread de sincronização com o servidor criada!%s\n", COLOR_CYAN, COLOR_RESET);

	while(1) {
		// DEBUG_PRINT("%sSYNC: Esperando na thread de sincronização%s\n", COLOR_CYAN, COLOR_RESET);
		usleep(SYNC_SLEEP);
		// DEBUG_PRINT("%sSYNC: Passou tempo definido, sincronizando!%s\n", COLOR_CYAN, COLOR_RESET);

		pthread_mutex_lock(&mutex_up_down_del_list);
		pthread_mutex_lock(&mutex_watcher);

    strcpy(buffer, ".");
    write_to_socket(ssl, buffer);

		bzero(buffer, BUFFER_SIZE);
		strcpy(buffer, S_SYNC_LOCAL);
		write_to_socket(ssl, buffer);
		synchronize_local(ssl, FALSE);

		pthread_mutex_unlock(&mutex_watcher);
		pthread_mutex_unlock(&mutex_up_down_del_list);
	}
}
