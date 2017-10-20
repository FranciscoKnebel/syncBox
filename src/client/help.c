#include "dropboxClient.h"

void help_help() {
  puts("Você já está aqui!");

  puts("Para ver todos os comandos, insira apenas help.");
}

void help_upload() {
  printf("COMANDO: \n %s ", COMMAND_UPLOAD);
  cprintf(ANSI_COLOR_GREEN, "<path/filename.ext>");

  puts("\nDESCRIÇÃO: ");
  puts("Envia o arquivo filename.ext para o servidor.");
  puts("e.g. upload /home/fulano/MyFolder/teste.txt");
}

void help_download() {
  printf("COMANDO: \n %s ", COMMAND_DOWNLOAD);
  cprintf(ANSI_COLOR_GREEN, "<filename.ext>");

  puts("\nDESCRIÇÃO: ");
  puts("Faz download do arquivo filename.ext do servidor para o diretório local.");
  puts("(de onde o servidor foi chamado).");
  puts("e.g. download mySpreadsheet.xlsx");
}

void help_listserver() {
  printf("COMANDO: \n %s ", COMMAND_LS);

  puts("\nDESCRIÇÃO: ");
  puts("Lista os arquivos salvos no servidor associados ao usuário.");
}

void help_listclient() {
  printf("COMANDO: \n %s ", COMMAND_LC);

  puts("\nDESCRIÇÃO: ");
  puts("Lista os arquivos salvos no diretório 'sync_dir_<nomeusuário>'.");
}

void help_sync() {
  printf("COMANDO: \n %s ", COMMAND_SYNC);

  puts("\nDESCRIÇÃO: ");
  puts("Cria o diretório 'sync_dir_<nomeusuário>' no /home do usuário.");
}

void help_credits() {
  printf("COMANDO: \n %s ", COMMAND_CREDITS);
  
  puts("\nDESCRIÇÃO: ");
  puts("Informações de autoria.");
}

void help_exit() {
  printf("COMANDO: \n %s ", COMMAND_EXIT);

  puts("\nDESCRIÇÃO: ");
  puts("Fecha conexão com o servidor.");
}

void help_undef(char* command) {
  printf("Comando '%s' não reconhecido.", command);
}
