void synchronize_local(SSL *sockid, int print);
void synchronize_server(SSL *sockid);

#define WATCHER_EVENT_SIZE (sizeof(struct inotify_event))
#define WATCHER_EVENT_BUF_LEN (1024 * (WATCHER_EVENT_SIZE))
void *watcher_thread(void* ptr_path);

#define SYNC_SLEEP 5000000 // microseconds
void* sync_devices_thread();
