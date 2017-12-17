#include "dropboxClient.h"

void command_upload(char* path) {
  pthread_mutex_lock(&mutex_up_down_del_list);
  if(check_connection()){
    send_file(path, TRUE);
  } else{
    reconnect_server();
    send_file(path, TRUE);
  }
  pthread_mutex_unlock(&mutex_up_down_del_list);
}

void command_download(char* path) {
  pthread_mutex_lock(&mutex_up_down_del_list);
  char cwd[MAXNAME*2];

  if (getcwd(cwd, sizeof(cwd)) == NULL) {
    perror("getcwd() error");
  }
  if(check_connection()){
    get_file(path, cwd);
  } else{
    reconnect_server();
    get_file(path, cwd);
  }
  pthread_mutex_unlock(&mutex_up_down_del_list);
}

void command_listserver() {
  pthread_mutex_lock(&mutex_up_down_del_list);
  if(check_connection()){
    list_server();
  } else{
    reconnect_server();
    list_server();
  }
  pthread_mutex_unlock(&mutex_up_down_del_list);
}

void command_listclient() {
  if(!fileExists(user.folder)) {
    printf("Error, User folder '%s' doesn't exist.\n", user.folder);
  } else {
    print_dir_content(user.folder);
  }
}

void command_getsyncdir() { // DEPRECATED get_sync_dir
  if(!fileExists(user.folder)) {
    if(mkdir(user.folder, 0777) != 0) {
      printf("Error creating user folder '%s'.\n", user.folder);
    }
  }

  char buffer[BUFFER_SIZE];
  strcpy(buffer, S_GETSYNCDIR);
  write_to_socket(ssl, buffer);

  printf("%sComando GET SYNC DIR depreciado na versão 0.0.2.%s\n", COLOR_RED, COLOR_RESET);

  // sincroniza pasta local com o servidor
  synchronize_local(ssl, TRUE);
  printf("Sincronização finalizada.\n");

  // sincroniza servidor com pasta local
  //synchronize_server(ssl);
}

void command_clear() {
  system("clear");
}

void command_help(char* command) {
  if(strcmp(command, COMMAND_HELP) == 0) {
    help_help();
  } else if(strcmp(command, COMMAND_UPLOAD) == 0) {
    help_upload();
  } else if(strcmp(command, COMMAND_DOWNLOAD) == 0) {
    help_download();
  } else if(strcmp(command, COMMAND_LS) == 0) {
    help_listserver();
  } else if(strcmp(command, COMMAND_LC) == 0) {
    help_listclient();
  } else if(strcmp(command, COMMAND_SYNC) == 0) {
    help_sync();
  } else if(strcmp(command, COMMAND_CREDITS) == 0) {
    help_credits();
  } else if(strcmp(command, COMMAND_EXIT) == 0) {
    help_exit();
  } else {
    help_undef(command);
  }
}

void command_credits() {
  printf("Trabalho original por:\n\tFrancisco Knebel\n\tLuciano Zancan\n");
  cprintf(COLOR_MAGENTA, "\nsyncBox - 2017\n");
}
