#include "dropboxUtil.h"

void *dir_content_thread(void *ptr) {
   struct dir_content *args = (struct dir_content *) ptr;

   get_dir_content(args->path, args->files, args->counter);

   pthread_exit(NULL);
   return NULL;
}

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
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

      pthread_mutex_lock(&lock);
      memcpy(&files[(*counter)], &newFile, sizeof(newFile));
      (*counter)++;
      pthread_mutex_unlock(&lock);
      
      int rc;
      pthread_t thread;
      if(entry->d_type == DT_DIR) { // Arquivo é um diretório
        struct dir_content args;
        args.path = malloc(sizeof(char) * MAXNAME * 2 + 1); // MAXNAME + / + MAXNAME

        sprintf(args.path, "%s/%s", newFile.path, newFile.name);
        args.files = files;
        args.counter = counter;

        if((rc = pthread_create(&thread, NULL, &dir_content_thread, (void *) &args))) {
          printf("Thread creation failed: %d\n", rc);
        }

        pthread_join(thread, NULL);
      }
    }
  }

  closedir(d);
  return 0;
}

int get_dir_content_file_info(char * path, FileInfo files[]) {
  struct d_file dfiles[MAXFILES];
  int counter = 0;
  get_dir_content(path, dfiles, &counter);

  for(int i = 0; i < counter; i++){
  	strcpy(&files[i].name, &dfiles[i].name);
  	getFileCreationTime(&dfiles[i].path, &files[i].last_modified);
  	getFileSize(&dfiles[i].path, &files[i].size);
  	getFileExtension(&dfiles[i].name, &files[i].extension);
  }

  return counter;
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
    printf("%s%s%s/%s\n", COLOR_YELLOW, files[i].path, COLOR_RESET, files[i].name);
  }
  printf("ELEMENTS FOUND: %d\n", counter);

  return 0;
}

void getFileCreationTime(char *path, char* last_modified) {
  struct stat attr;
  stat(path, &attr);
  strcpy(last_modified, ctime(&attr.st_mtime));
}

void getFileSize(char *path, int* size) {
  struct stat attr;
  stat(path, &attr);
  size = attr.st_size;
}

void getFileExtension(const char *filename, char* extension) {
  const char *dot = strrchr(filename, '.');
  if(!dot || !strcmp(dot, filename)) {
    strcpy(extension, "");
  } else {
  	strcpy(extension, dot+1);
  }
}

void getLastStringElement(char filename[], char* string, const char *separator){
	string = strtok(string, separator);

	while (string) {
    //printf("element: %s\n", string);
    strcpy(filename, string);
    string = strtok(NULL, separator);
	}
	//printf("final: %s\n", filename);
}
