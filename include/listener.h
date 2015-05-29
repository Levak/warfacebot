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
	unsigned int last;
};

struct active_listeners_t *create_listeners ( );

char *add_listener ( struct active_listeners_t *plisteners, char *nick );

int remove_listener ( struct active_listeners_t *plisteners, char *nick );

int search_listener ( struct active_listeners_t *plisteners, char *nick );

void print_listeners ( struct active_listeners_t *plisteners );