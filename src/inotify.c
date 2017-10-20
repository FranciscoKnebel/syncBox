#include "util.h"
#include "dropboxUtil.h"

#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (1024 * ( EVENT_SIZE + 16 ))

int main_inotify(char* path) {
  int fd, wd;
  int length, i = 0;
  char buffer[EVENT_BUF_LEN];

  /* creating the INOTIFY instance */
  fd = inotify_init();

  /* checking for error */
  if(fd < 0) {
    perror("inotify_init");
  }

  /* adding the directory into watch list. Here, the suggestion is to validate the existence of the directory before adding into monitoring list. */
  wd = inotify_add_watch(fd, path, IN_CLOSE_WRITE | IN_CREATE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO);

  /* read to determine the event change happens on “/tmp” directory. Actually this read blocks until the change event occurs */
  length = read(fd, buffer, EVENT_BUF_LEN);

  /* checking for error */
  if (length < 0) {
    perror("read");
  }

  char path2[MAXNAME];
  /* actually read return the list of change events happens. Here, read the change event one by one and process it accordingly. */
  while (i < length) {
    struct inotify_event* event = (struct inotify_event *) &buffer[i];

    if (event->len) {
      if (event->mask & IN_CLOSE_WRITE || event->mask & IN_CREATE || event->mask & IN_MOVED_TO) {
        //strcpy(path2, user.folder);
        strcat(path2, "/");
        strcat(path2, event->name);

        if(fileExists(path2) && (event->name[0] != '.')) {
          // request to upload file (path2);
        }
      } else if (event->mask & IN_DELETE || event->mask & IN_MOVED_FROM) {
        //strcpy(path2, user.folder);
        strcat(path2, "/");
        strcat(path2, event->name);

        if(event->name[0] != '.') {
          // request to delete file (path2);
        }
      }
    }

    i += EVENT_SIZE + event->len;
  }

  /* removing the “/tmp” directory from the watch list. */
  inotify_rm_watch(fd, wd);

  /* closing the INOTIFY instance */
  close(fd);

  return 0;
}
