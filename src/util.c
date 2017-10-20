#include "util.h"

#include <stdio.h>

int cprintf(char* ansicode, char* string) {
	return printf("%s%s"ANSI_COLOR_RESET, ansicode, string);
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

char* getUserName()
{
  uid_t uid = geteuid();
  struct passwd *pw = getpwuid(uid);

	if (pw) {
    return pw->pw_name;
  }
  return "";
}

char* getUserHome()
{
  struct passwd *pw = getpwuid(geteuid());

	if (pw) {
    return pw->pw_dir;
  }
  return "";
}
