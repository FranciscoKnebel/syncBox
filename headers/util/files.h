#ifndef util_h
#define util_h
  #include "dropboxUtil.h"
#endif

int get_dir_content(char * path, struct d_file files[], int* counter);
int get_dir_file_info(char * path, FileInfo files[]);
int get_all_entries(char * path, struct d_file files[]);
int print_dir_content(char * path);
int getFileModifiedTime(char *path, char* last_modified);
void getFileExtension(const char *filename, char* extension);
int  getFileSize(char *path);
void getLastStringElement(char filename[], char* string, const char *separator);
time_t getTime(char* last_modified);
int setModTime(char* path, time_t mod_time);
int older_file(char* last_modified_file_1, char* last_modified_file_2);

int getFilesize(FILE* pFile);
int fileExists_stat(char* pathname, struct stat* st);
int fileExists(char* pathname);
int getFileIndex(char* filename, FileInfo file_info[]);
