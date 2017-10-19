#include "util.h"
#include "dropboxUtil.h"
#include <assert.h>

int main(int argc, char *argv[]) {
  char path[MAXNAME];
  strcpy(path, argv[1]);

  int count1 = 0, count2 = 0;
  FILE* pFile1, *pFile2;

  // abre um arquivo e verifica o seu tamanho.
  pFile1 = fopen(path, "rb");
  if(pFile1) {
    assert(getFilesize(pFile1));
  } else {
    printf("Erro ao abrir %s.", path);
  }
  fclose(pFile1);

  // abre um arquivo e verifica o seu tamanho, duas vezes, comparando resultado.
  pFile1 = fopen(path, "rb");
  if(pFile1) {
    count1 = getFilesize(pFile1);
  } else {
    printf("Erro ao abrir %s.", path);
  }
  fclose(pFile1);

  pFile1 = fopen(path, "rb");
  if(pFile1) {
    count2 = getFilesize(pFile1);
  } else {
    printf("Erro ao abrir %s.", path);
  }
  fclose(pFile1);

  assert(count1 == count2);

  // abre um arquivo duas vezes separadamente, verifica se o tamanho é o mesmo.
  pFile1 = fopen(path, "rb");
  pFile2 = fopen(path, "rb");
  if(pFile1 && pFile2) {
    count1 = getFilesize(pFile1);
    count2 = getFilesize(pFile2);

    assert(count1 == count2);
  } else {
    printf("Erro ao abrir %s.", path);
  }
  fclose(pFile1);
  fclose(pFile2);

  return 0;
}
