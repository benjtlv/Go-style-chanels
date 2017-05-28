struct channel;

/* flags */
#define CHANNEL_PROCESS_UNSHARED 0
#define CHANNEL_PROCESS_SHARED 1
#define CHANNEL_OPEN 10
#define CHANNEL_CLOSED 0

struct channel *sync_channel_create(int);
struct channel *async_channel_create(int, int);
struct channel *local_channel_create(int, int);
struct channel *global_channel_create(int, int, char*);
struct channel *channel_create(int, int, int);
struct channel *named_channel_create(int, int, char*);
void channel_destroy(struct channel*);
int channel_send(struct channel*, const void*);
int channel_close(struct channel*);
int channel_recv(struct channel*, void*);

