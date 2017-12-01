typedef struct file_info
{
  char name[MAXNAME];
  char extension[MAXNAME];
  char last_modified[MAXNAME];
  int size;
} FileInfo;

typedef struct d_file
{
  char path[MAXNAME];
  char name[MAXNAME];
} DFile;

typedef struct dir_content {
  char* path;
  struct d_file* files;
  int* counter;
} DirContent;
