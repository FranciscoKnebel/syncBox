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
  cprintf(ANSI_COLOR_GREEN, COMMAND_HELP);
  return 0;
}
