#include <listener.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <textcolor.h>

#define	LISTENER(i)		plisteners->listeners[(i)]

void create_listeners ( )
{
	plisteners = malloc ( sizeof *plisteners );
	unsigned int i = 0;
	for ( ; i != MAXLISTENERS; ++i )
	{
		LISTENER ( i ).active = LISTENER ( i ).start = 0;
		LISTENER ( i ).nick = NULL;
	}
}

char *add_listener ( char *nick )
{
	unsigned int i = 0, oldest = 0, maxtime = 0;
	for ( ; i != MAXLISTENERS; ++i )
	{
		if ( LISTENER(i).nick && !strcmp ( LISTENER(i).nick, nick ) )
		{
			int was_active = LISTENER ( i ).active;
			LISTENER(i).active = 1;
			LISTENER(i).start = time ( NULL );
			if ( !was_active )
				return NULL;
			return nick;
		}
		else if ( !LISTENER(i).active )
		{
			char *result = LISTENER(i).nick;
			LISTENER(i).nick = strdup ( nick );
			LISTENER(i).start = time ( NULL );
			LISTENER(i).active = 1;
			return result;
		}
		if ( LISTENER(i).start > maxtime )
		{
			oldest = i;
			maxtime = LISTENER(i).start;
		}
	}
	char *result = LISTENER( oldest ).nick;
	LISTENER( oldest ).nick = strdup ( nick );
	LISTENER ( oldest ).active = 1;
	LISTENER ( oldest ).start = time ( NULL );
	return result;
}

int remove_listener ( char *nick )
{
	if ( !plisteners )
		create_listeners ( );
	unsigned int i = 0;
	for ( ; i != MAXLISTENERS; ++i )
		if ( LISTENER ( i ).active && !strcmp ( LISTENER ( i ).nick, nick ) )
			return !( LISTENER ( i ).active = 0 );
	return 0;
}

int is_active_listener ( char *nick )
{
	unsigned int i = 0;
	for ( ; i != MAXLISTENERS; ++i )
	{
		if ( !LISTENER ( i ).nick )
			continue;
		if ( LISTENER ( i ).active && !strcmp ( LISTENER ( i ).nick, nick ) )
			return 1;
	}
	return 0;
}

void print_listeners ( )
{
	unsigned int i = 0;
	for ( ; i != MAXLISTENERS; ++i )
		printf ( "%-16s %-4d %u\n", LISTENER ( i ).nick,
		LISTENER ( i ).active, LISTENER ( i ).start );
}

#undef LISTENER