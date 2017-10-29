#include "dropboxClient.h"

void command_upload(char* path) {
  send_file(path, TRUE);
}

void command_download(char* path) {
  char cwd[MAXNAME*2];

  if (getcwd(cwd, sizeof(cwd)) != NULL) {
    get_file(path, cwd);
  } else {
    perror("getcwd() error");
  }
}

void command_listserver() {
  return list_server();
}

void command_listclient() {
  if(!fileExists(user.folder)) {
    printf("Error, User folder '%s' doesn't exist.\n", user.folder);
  } else {
    print_dir_content(user.folder);
  }
}

void command_getsyncdir() {
  if(!fileExists(user.folder)) {
    if(mkdir(user.folder, 0777) != 0) {
      printf("Error creating user folder '%s'.\n", user.folder);
    }
  }

  // sincroniza pasta local com o servidor
  sync_client();
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
  printf("Trabalho original por:\n");
  printf("\nFrancisco Knebel \t Gabriel Pittol\nLuciano Zancan   \t Matheus Krause\n");
  cprintf(COLOR_MAGENTA, "\nsyncBox - 2017\n");
}
