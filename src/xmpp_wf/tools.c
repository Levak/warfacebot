/**
 * WarfaceBot, a blind XMPP client for Warface (FPS)
 * Copyright (C) 2015, 2016 Levak Borok <levak92@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <wb_tools.h>
#include <wb_session.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_PLAIN_QUERY_SIZE 256

void LOGPRINT ( const char *fmt, ... )
{
	int need_hack = ( rl_readline_state & RL_STATE_READCMD ) > 0;
	char *saved_line;
	int saved_point;
	if ( need_hack )
	{
		saved_point = rl_point;
		saved_line = rl_copy_text ( 0, rl_end );
		rl_save_prompt ( );
		rl_replace_line ( "", 0 );
		rl_redisplay ( );
	}

	va_list args;
	va_start ( args, fmt );

	printf ( KWHT BOLD "[%s]  " KRST, get_timestamp ( ) );
	vprintf ( fmt, args );
	printf ( KRST );

	if ( session.fLog )
	{
		fflush ( session.fLog );
		fprintf ( session.fLog, KWHT BOLD "[%s]  " KRST, get_timestamp ( ) );
		vfprintf ( session.fLog, fmt, args );
		fprintf ( session.fLog, KRST );
		fflush ( session.fLog );
	}

	va_end ( args );

	if ( need_hack )
	{
		rl_restore_prompt ( );
		rl_replace_line ( saved_line, 0 );
		rl_point = saved_point;
		rl_redisplay ( );
		free ( saved_line );
	}
}

char *wf_get_query_content ( const char *msg )
{
	char *compressedData = strstr ( msg, "compressedData='" );

	if ( !compressedData )
		return get_info ( msg, "urn:cryonline:k01'>", "</query>", NULL );

	compressedData += sizeof ( "compressedData='" ) - 1;

	char *originalSize = strstr ( msg, "originalSize='" );

	if ( !originalSize )
		return NULL;

	originalSize += sizeof ( "originalSize='" ) - 1;

	size_t outsize = strtol ( originalSize, NULL, 10 );
	size_t insize = strstr ( compressedData, "'" ) - compressedData + 1;

	char *ret = zlibb64decode ( compressedData, insize, outsize );

#ifdef DEBUG
	if ( ret != NULL )
	{
		printf ( "##(%3u/%3u)-< ", (unsigned) outsize, (unsigned) insize );
		printf ( "\033[1;36m%s\033[0m\n", ret );
	}
#endif

	if ( session.fDebug )
	{
		fprintf ( session.fDebug, KWHT BOLD "[%s]  " KRST, get_timestamp ( ) );
		fprintf ( session.fDebug, "##(%3u/%3u)-< ", (unsigned) outsize, (unsigned) insize );
		fprintf ( session.fDebug, KCYN BOLD "%s\n" KRST, ret );
	}

	return ret;
}

char *wf_compress_query ( const char *iq )
{
	if ( iq == NULL )
		return NULL;

	size_t total_size = strlen ( iq );

	if ( total_size < MAX_PLAIN_QUERY_SIZE )
		return strdup ( iq );

	char *query = get_info ( iq, "urn:cryonline:k01'>", "</query>", NULL );
	char *prologue = get_info ( iq, "<", "urn:cryonline:k01'>", NULL );
	char *epilogue = get_info ( iq, "</query>", "</iq>", NULL );

	if ( query == NULL || prologue == NULL || epilogue == NULL )
		return strdup ( iq );

	size_t osize = strlen ( query );
	char *compressed = zlibb64encode ( query, osize );
	char *query_name = get_info_first ( query, "<", " />", NULL );

	char *ret = NULL;
	FORMAT ( ret,
			 "<%surn:cryonline:k01'>"
			 "<data query_name='%s'"
			 " compressedData='%s'"
			 " originalSize='%u'"
			 "/>"
			 "</query>%s</iq>",
			 prologue, query_name, compressed, osize, epilogue );

	free ( query );
	free ( prologue );
	free ( epilogue );
	free ( compressed );
	free ( query_name );

	return ret;
}

char *wf_decompress_query ( const char *iq )
{
	if ( iq == NULL )
		return NULL;

	return strdup ( iq );
}