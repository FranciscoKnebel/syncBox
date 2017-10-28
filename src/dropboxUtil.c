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

int get_dir_file_info(char * path, FileInfo files[]) {
  struct d_file dfiles[MAXFILES];
  int counter = 0;
  get_dir_content(path, dfiles, &counter);
  char path_file[MAXNAME*2];
  for(int i = 0; i < counter; i++){
  	strcpy(&files[i].name, &dfiles[i].name);
        sprintf(path_file, "%s/%s", dfiles[i].path, dfiles[i].name);
  	getFileModifiedTime(&path_file, &files[i].last_modified);
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

void getFileModifiedTime(char *path, char* last_modified) {
  struct stat attr;
  stat(path, &attr);
  strftime(last_modified, 20, "%Y.%m.%d %H:%M:%S", localtime(&(attr.st_mtime)));
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

void getLastStringElement(char filename[], char* string, const char *separator) {
	string = strtok(string, separator);

	while (string) {
    //printf("element: %s\n", string);
    strcpy(filename, string);
    string = strtok(NULL, separator);
	}
	//printf("final: %s\n", filename);
}


time_t getTime(char* last_modified){
   
   time_t result = 0;
   
   int year = 0, month = 0, day = 0, hour = 0, min = 0, sec = 0;;
   
   if (sscanf(last_modified, "%4d.%2d.%2d %2d:%2d:%2d", &year, &month, &day, &hour, &min, &sec) == 6) {
       struct tm breakdown = {0};
       breakdown.tm_year = year - 1900; /* years since 1900 */
       breakdown.tm_mon = month - 1;
       breakdown.tm_mday = day;
       breakdown.tm_hour = hour;
       breakdown.tm_min = min;
       breakdown.tm_sec = sec;
     
       if ((result = mktime(&breakdown)) == (time_t)-1) {
          fprintf(stderr, "Could not convert time input to time_t\n");
          return EXIT_FAILURE;
       }
        
       return result;
   }
   else {
      fprintf(stderr, "The input was not a valid time format\n");
      return EXIT_FAILURE;
   }
}

int older_file(char* last_modified_file_1, char* last_modified_file_2){ // return 1 if file2 is older than the modified date of 
                                                                        // file 1 else return 0
	time_t time_file_1 = getTime(last_modified_file_1);
        time_t time_file_2 = getTime(last_modified_file_2);
	if(difftime(time_file_1, time_file_2) > 0){
		return 1;
	}
	return 0;
	
}
