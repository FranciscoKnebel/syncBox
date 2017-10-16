#include "dropboxClient.h"

int command_upload(char* path) {
  cprintf(ANSI_COLOR_GREEN, COMMAND_UPLOAD);
  return 0;
}

int command_download(char* path) {
  cprintf(ANSI_COLOR_GREEN, COMMAND_DOWNLOAD);
  return 0;
}

int command_listserver() {
  cprintf(ANSI_COLOR_GREEN, COMMAND_LS);
  return 0;
}

int command_listclient() {
  cprintf(ANSI_COLOR_GREEN, COMMAND_LC);
  return 0;
}

int command_getsyncdir() {
  cprintf(ANSI_COLOR_GREEN, COMMAND_SYNC);
  return 0;
}

int command_help(char* command) {
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
  } else if(strcmp(command, COMMAND_EXIT) == 0) {
    help_exit();
  } else {
    help_undef(command);
  }

  return 0;
}
