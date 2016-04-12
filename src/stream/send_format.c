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
#include <stdlib.h>
#include <stdarg.h>
#include <wb_stream.h>

#define FORMAT_STRING_SIZE 255
void send_stream_format ( int fd, const char *fmt, ... )
{
	unsigned int len;
	va_list ap;
	char s[ FORMAT_STRING_SIZE ];

	va_start ( ap, fmt );
	len = vsnprintf ( s, FORMAT_STRING_SIZE, fmt, ap );
	va_end ( ap );

	if ( len >= FORMAT_STRING_SIZE )
	{
		char *s2 = malloc ( len + 1 );

		va_start ( ap, fmt );
		vsprintf ( s2, fmt, ap );
		va_end ( ap );

		send_stream ( fd, s2, len );
		free ( s2 );
	}
	else
		send_stream ( fd, s, len );

	flush_stream ( fd );
}