#include "dropboxClient.h"

void command_upload(char* path) {
  return send_file(path);
}

void command_download(char* path) {
  return get_file(path);
}

void command_listserver() {
  // Lista os arquivos salvos no servidor associados ao usuário.
  // pede lista para o servidor e imprime resultado.
}

void command_listclient() {
  // lista os arquivos salvos no diretório "sync_dir_<nomeusuario>".
  // verifica se o diretório sync_dir_<nomeusuario> existe
    // se não, retorna erro.

  // retorna os arquivos encontrados no diretório.
}

void command_getsyncdir() {
  // verifica se o diretório /home/user/sync_dir_<nomeusuario> existe
    // se não, cria.
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
  } else if(strcmp(command, COMMAND_EXIT) == 0) {
    help_exit();
  } else {
    help_undef(command);
  }
}
