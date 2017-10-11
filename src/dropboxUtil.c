#include "dropboxUtil.h"

int show_dir_content(char * path, struct dirent ** files) {
  DIR * d = opendir(path);
  if(d == NULL) {
    return;
  }

  struct dirent * dir;
  while ((dir = readdir(d)) != NULL) {
    if(dir-> d_type != DT_DIR) { // Arquivo não é um diretório
      printf("%s%s\n", BLUE, dir->d_name);
      // add to files
    } else if(dir -> d_type == DT_DIR && strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0 ) {
      // Arquivo é um diretório
      printf("%s%s\n", GREEN, dir->d_name);

      char d_path[255];
      sprintf(d_path, "%s/%s", path, dir->d_name);
      show_dir_content(d_path);
    }
  }

  closedir(d);
  return 0;
}

int print_dir_content(char * path) {
  struct dirent ** files;

  show_dir_content(path, files);

  /*
  TO DO

  print file names for debugging
  remove prints from show_dir_content
  */

  return 0;
}
