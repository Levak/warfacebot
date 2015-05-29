#define MAXLISTENERS	4

struct listener_t
{
	char *nick;
	int active;
	unsigned int start;
};

struct active_listeners_t
{
	struct listener_t listeners[MAXLISTENERS];
};

extern struct active_listeners_t *plisteners;

void create_listeners ( );

char *add_listener ( char *nick );

int remove_listener ( char *nick );

int is_active_listener ( char *nick );

void print_listeners ( );