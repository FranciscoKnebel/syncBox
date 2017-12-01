#include "dropboxUtil.h"

int cprintf(char* ansicode, char* string) {
	return printf("%s%s"COLOR_RESET, ansicode, string);
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
