#ifndef FILEUTIL_H
#define FILEUTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <assert.h>

int read_file(char *content,int size,char *file_name);
int remove_file(char *file_name);
int write_file(char *content,int size,char *file_name);
int get_file_extension(char *file_name,char **file_extension);
int get_last_modification_time(char *file_name,char **last_modfication);

#endif