/**
 * WarfaceBot, a blind XMPP client for Warface (FPS)
 * Copyright (C) 2015 Levak Borok <levak92@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "def.h"

#include <stdio.h>
#include <string.h>

#include <sys/types.h>

#ifdef __MINGW32__
# include <Winsock.h>
#else
# include <sys/socket.h>
#endif

void send_stream(int fd, char *msg, uint32_t msg_size)
{
    ssize_t wrote_size = 0;
    struct stream_hdr hdr;

    hdr.magic = 0xFEEDDEAD;
    hdr.xor = 0;
    hdr.len = msg_size;

    send(fd, &hdr, sizeof (hdr), MSG_MORE);
    wrote_size = send(fd, msg, msg_size, MSG_MORE);

#ifdef DEBUG
    printf("--(%3u/%3u)-> ", (unsigned) wrote_size, msg_size);
    printf("\033[1;31m%s\033[0m\n", msg);
#endif
}

void send_stream_ascii(int fd, char *msg)
{
    send_stream(fd, msg, strlen(msg));
}

void flush_stream(int fd)
{
    send(fd, "", 0, 0);
}
