#include "util.h"

#include <stdio.h>

int cprintf(char* ansicode, char* string) {
	return printf("%s%s"ANSI_COLOR_RESET, ansicode, string);
}
