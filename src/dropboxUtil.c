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

int get_all_entries(char * path, struct d_file files[]) {
  int counter = 0;

  return get_dir_content(path, files, &counter);
}

int print_dir_content(char * path) {
  struct d_file files[MAXFILES];
  int counter = 0;

  get_dir_content(path, files, &counter);

  printf("Number of files: %d\n", counter);
  for (int i = 0; i < counter; i++) {
    printf("%s%s%s/%s\n", COLOR_YELLOW, files[i].path, COLOR_RESET, files[i].name);
  }
  printf("Number of files: %d\n", counter);

  return 0;
}

int get_dir_file_info(char * path, FileInfo files[]) {
  struct d_file dfiles[MAXFILES];
  char path_file[MAXNAME*2 + 1];
  int counter = 0;
  get_dir_content(path, dfiles, &counter);

  for(int i = 0; i < counter; i++) {
  	strcpy(files[i].name, dfiles[i].name);
    sprintf(path_file, "%s/%s", dfiles[i].path, dfiles[i].name);
  	getFileModifiedTime((char*) &path_file, (char*) &files[i].last_modified);
    getFileExtension(dfiles[i].name, (char*) &files[i].extension);
  	files[i].size = getFileSize(dfiles[i].path);
  }

  return counter;
}

int getFileModifiedTime(char *path, char* last_modified) {
  struct stat attr;
  int status = stat(path, &attr);
  if(status != 0) {
    return status;
  }

  time_t mod_time = attr.st_mtime;
  struct tm *timeptr = localtime(&mod_time);
  if(timeptr->tm_isdst > 0) { // is daylight saving time
    mod_time -= 3600; // - 1 hour
  }

  strftime(last_modified, MAXNAME, "%Y.%m.%d %H:%M:%S", localtime(&mod_time));
  return 0;
}

int getFileSize(char *path) {
  struct stat attr;
  stat(path, &attr);

  return attr.st_size;
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
  char* str = malloc(sizeof(char) * MAXNAME);
  strcpy(str, string);

	str = strtok(str, separator);

	while (str) {
  	strcpy(filename, str);
  	str = strtok(NULL, separator);
	}
}

time_t getTime(char* last_modified){
  time_t result = 0;
  int year = 0, month = 0, day = 0, hour = 0, min = 0, sec = 0;

  if (sscanf(last_modified, "%4d.%2d.%2d %2d:%2d:%2d", &year, &month, &day, &hour, &min, &sec) == 6) {
    struct tm breakdown = { 0 };
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
    //DEBUG_PRINT("Time transformado: %s", ctime(&result));
    return result;
  } else {
    fprintf(stderr, "The input was not a valid time format: %s\n", last_modified);
    return EXIT_FAILURE;
  }
}

int setModTime(char* path, time_t mod_time) {
  struct utimbuf fileTimes;
  fileTimes.actime = mod_time;
  fileTimes.modtime = mod_time;

  return utime(path, &fileTimes);
}

int older_file(char* last_modified_file_1, char* last_modified_file_2) {
	time_t time_file_1 = getTime(last_modified_file_1);
  time_t time_file_2 = getTime(last_modified_file_2);

  // return 1 if file2 is older than the modified date of file 1
  double difference = difftime(time_file_1, time_file_2);
  DEBUG_PRINT("Dif: %f - FILE1: %ld - FILE2: %ld\n", difference, time_file_1, time_file_2);
	if(difference > 0) {
    DEBUG_PRINT("File2 is older than File1.\n");
		return 1;
	} else if (difference == 0) {
    DEBUG_PRINT("File1 is the same age as File2.\n");
    return -1;
  }
  return 0;
}

int cprintf(char* ansicode, char* string) {
	return printf("%s%s"COLOR_RESET, ansicode, string);
}

int getFilesize(FILE* pFile) {
	int size;

	fseek(pFile, 0, SEEK_END);
	size = ftell(pFile); // dá pra usar stat ou fstat também, mas o comportamento é diferente. stat retorna o total dos blocos alocados, não a quantidade real em bytes do arquivo.
	rewind(pFile);

	return size;
}

int fileExists_stat(char* pathname, struct stat* st) {
	if (stat(pathname, st) == -1)
		return 0; // False
	return 1;
}

int fileExists(char* pathname) {
	struct stat st = { 0 };

	return fileExists_stat(pathname, &st);
}

int getFileIndex(char* filename, FileInfo file_info[]){
  int index = 0;
  for(int i = 0; i < MAXFILES; i++){
    if(strcmp(filename, file_info[i].name) == 0){
      index = i;
      return index;
    }
  }
  return index;
}

char* getUserName() {
  uid_t uid = geteuid();
  struct passwd *pw = getpwuid(uid);

	if (pw) {
    return pw->pw_name;
  }
  return "";
}

char* getUserHome() {
  struct passwd *pw = getpwuid(geteuid());

	if (pw) {
    return pw->pw_dir;
  }
  return "";
}

int readToFile(FILE* pFile, int file_size, int sockid) {
  int bytes_written = 0;
  int bytes_read = 0;

  char buffer[BUFFER_SIZE];

  while(bytes_written < file_size) {
    bytes_read = read(sockid, buffer, BUFFER_SIZE); // recebe arquivo no buffer
    if (bytes_read < 0) {
      DEBUG_PRINT("ERROR reading from socket\n");
    }

    if(bytes_read > (file_size - bytes_written)) {
      fwrite(buffer, sizeof(char), (file_size - bytes_written), pFile);
      bytes_written += sizeof(char) * (file_size - bytes_written);
    } else {
      fwrite(buffer, sizeof(char), bytes_read, pFile);
      bytes_written += sizeof(char) * bytes_read;
    }
    // DEBUG_PRINT("leu buffer %d bytes - Total: %d / Escritos: %d / Sobrando: %d\n", bytes_read, file_size, bytes_written, (file_size - bytes_written));
  }

  return bytes_written;
}
