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
