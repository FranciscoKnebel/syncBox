#include <stdio.h>
#include <string.h>

#include "dropboxUtil.h"

int get_dir_content(char * path, struct d_file files[], int* counter) {
  DIR * d = opendir(path);
  if(d == NULL) {
    return FILE_NOT_FOUND;
  }

  struct dirent * entry;
  while (((entry = readdir(d)) != NULL) && (*counter) < MAXFILES) {
    if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
      struct d_file newFile;
      strcpy(newFile.path, path);
      strcpy(newFile.name, entry->d_name);

      memcpy(&files[(*counter)++], &newFile, sizeof(newFile));

      if(entry->d_type == DT_DIR) { // Arquivo é um diretório
        char dpath[MAXPATH];
        sprintf(dpath, "%s/%s", newFile.path, newFile.name);

        get_dir_content(dpath, files, counter);
      }
    }
  }

  closedir(d);
  return 0;
}

int get_all_entries(char * path, struct d_file files[]) {
  int counter = 0;

  return get_dir_content(path, files, &counter);
}

int print_dir_content(char * path) {
  struct d_file files[MAXFILES];
  int counter = 0;

  get_dir_content(path, files, &counter);

  printf("ELEMENTS FOUND: %d\n", counter);
  for (int i = 0; i < counter; i++) {
    printf("%s%s%s/%s\n", ANSI_COLOR_YELLOW, files[i].path, ANSI_COLOR_RESET, files[i].name);
  }

  return 0;
}
