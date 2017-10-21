#include "dropboxUtil.h"
#include "client/watcher.h"

int main(int argc, char *argv[]) {
  int rc;
  pthread_t thread;

  if((rc = pthread_create(&thread, NULL, watcher_thread, (void *) argv[1]))) {
    printf("Thread creation failed: %d\n", rc);
  }

  pthread_join(thread, NULL);
  return 0;
}
