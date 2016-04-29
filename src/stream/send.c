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

#include "def.h"

#include <stdio.h>
#include <string.h>

#include <sys/types.h>

#ifdef __MINGW32__
#include <Winsock.h>
#else
#include <sys/socket.h>
#endif

#include "wb_stream.h"
#include <wb_session.h>
#include <wb_tools.h>

#ifdef USE_TLS
#define SEND(Fd, Buf, Size) tls_send((Fd), (Buf), (Size))
#else
#define SEND(Fd, Buf, Size) send((Fd), (Buf), (Size), MSG_MORE)
#endif

#include <wb_xmpp_wf.h>

void send_stream ( int fd, const char *msg, uint32_t msg_size )
{
	char *compressed = wf_compress_query ( msg );
	char *buffer = NULL;

#ifdef DEBUG
	if ( crypt_is_ready ( ) )
		printf ( "%s==(%3u)=> ", compressed ? "##" : "==", msg_size );
	else
		printf ( "%s--(%3u)-> ", compressed ? "##" : "--", msg_size );
	printf ( "\033[1;31m%s\033[0m\n", msg );
#endif

	if ( session.fDebug )
	{
		fflush ( session.fDebug );
		fprintf ( session.fDebug, KWHT BOLD "[%s]  " KRST, get_timestamp ( ) );
		if ( crypt_is_ready ( ) )
			fprintf ( session.fDebug, "%s==(%3u)=> ", compressed ? "##" : "==", msg_size );
		else
			fprintf ( session.fDebug, "%s--(%3u)-> ", compressed ? "##" : "--", msg_size );
		fprintf ( session.fDebug, KRED BOLD "%s\n" KRST, msg );
		fflush ( session.fDebug );
	}

	if ( compressed != NULL && strstr ( msg, "to='k01.warface'" ) == NULL )
	{
		msg_size = strlen ( compressed );
		buffer = compressed;
	}
	else
	{
		buffer = strdup ( msg );
		free ( compressed );
		compressed = buffer;
	}

#ifdef USE_PROTECT
	struct stream_hdr hdr;

	hdr.magic = STREAM_MAGIC;
	hdr.se = SE_PLAIN;
	hdr.len = msg_size;

	if ( crypt_is_ready ( ) )
	{
		hdr.se = SE_ENCRYPTED;
		crypt_encrypt ( (uint8_t *) buffer, msg_size );
	}

	SEND ( fd, &hdr, sizeof ( hdr ) );
#endif /* USE_PROTECT */

	SEND ( fd, buffer, msg_size );

	free ( compressed );
}

void send_stream_ascii ( int fd, const char *msg )
{
	send_stream ( fd, msg, strlen ( msg ) );
}

void send_stream_ack ( int fd )
{
	struct stream_hdr hdr;

	hdr.magic = STREAM_MAGIC;
	hdr.se = SE_CLIENT_ACK;
	hdr.len = 0;

#ifdef DEBUG
	printf ( "----()-> ACK KEY\n" );
#endif

	SEND ( fd, &hdr, sizeof ( hdr ) );

	flush_stream ( fd );
}

void flush_stream ( int fd )
{
	send ( fd, "", 0, 0 );
}