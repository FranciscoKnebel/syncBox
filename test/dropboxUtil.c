#include "dropboxUtil.h"
#include <assert.h>

int main(int argc, char *argv[]) {
  int counter = 0;
  char path[MAXNAME];
  struct d_file files[MAXFILES];
  strcpy(path, argv[1]);

  // Pega conteúdo recursivo do diretório
  assert(get_dir_content(path, files, &counter) == 0); // returned successfuly.
  assert(counter >= 0 && counter <= MAXFILES); // encontrou de 0 até MAXFILES entradas no diretório.

  // Pega todas entradas, sem se preocupar com o contador
  assert(get_all_entries(argv[1], files) == 0);

  // Print de todas as entradas, sem se preocupar com o contador ou manter os arquivos
  assert(print_dir_content(path) == 0);

  return 0;
}
