#include <listener.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <textcolor.h>

#define	LISTENER(i)		plisteners->listeners[(i)]

struct active_listeners_t *create_listeners ( )
{
	struct active_listeners_t *plisteners = malloc ( sizeof *plisteners );
	plisteners->last = 0;
	unsigned int i = 0;
	for ( ; i != MAXLISTENERS; ++i )
	{
		LISTENER ( i ).active = LISTENER ( i ).start = 0;
		LISTENER ( i ).nick = NULL;
	}
	return plisteners;

}

char *add_listener ( struct active_listeners_t *plisteners, char *nick )
{
	unsigned int i = plisteners->last, oldest = i, maxtime = 0;
	do 
	{
		if ( LISTENER(i).nick && !strcmp ( LISTENER(i).nick, nick ) )
		{
			LISTENER(i).active = 1;
			LISTENER(i).start = time ( NULL );
			//print_listeners ( plisteners );
			return NULL;
		}
		else if ( !LISTENER(i).active )
		{
			char *result = LISTENER(i).nick;
			LISTENER(i).nick = strdup ( nick );
			LISTENER(i).start = time ( NULL );
			LISTENER(i).active = 1;
			//print_listeners ( plisteners );
			return result;
		}
		if ( LISTENER(i).start > maxtime )
		{
			oldest = i;
			maxtime = LISTENER(i).start;
		}
		i = ( i + 1 ) % MAXLISTENERS;
	} while (i != plisteners->last);
	char *result = LISTENER( oldest ).nick;
	LISTENER( oldest ).nick = strdup ( nick );
	LISTENER ( oldest ).active = 1;
	LISTENER ( oldest ).start = time ( NULL );
	return result;
}

int remove_listener ( struct active_listeners_t *plisteners, char *nick )
{
	unsigned int i = 0;
	for ( ; i != MAXLISTENERS; ++i )
	{
		if ( LISTENER ( i ).active && !strcasecmp ( LISTENER ( i ).nick, nick ) )
		{
			LISTENER ( i ).active = 0;
			return 1;
		}
	}
	return 0;
}

int search_listener ( struct active_listeners_t *plisteners, char *nick )
{
	unsigned int i = 0;
	for ( ; i != MAXLISTENERS; ++i )
	{
		if ( !LISTENER ( i ).nick )
			continue;
		int cmp = strcmp ( LISTENER ( i ).nick, nick );
		if ( LISTENER ( i ).active && !cmp )
			return i;
	}
	return -1;
}

void print_listeners ( struct active_listeners_t *plisteners )
{
	unsigned int i = 0;
	for ( ; i != MAXLISTENERS; ++i )
		printf ( "%-16s %-4d %u\n", LISTENER ( i ).nick,
		LISTENER ( i ).active, LISTENER ( i ).start );
}