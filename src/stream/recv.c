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
#include <stdlib.h>
#include <string.h>

#ifdef __MINGW32__
# include <Winsock.h>
#else
# include <sys/socket.h>
#endif

char *read_stream_keep(int fd)
{
    char *msg;
    struct stream_hdr hdr;

    if (recv(fd, &hdr, sizeof (hdr), 0) != sizeof (hdr))
        return NULL;

    if (hdr.magic != 0xFEEDDEAD)
        return NULL;

    msg = calloc(hdr.len + 1, 1);
    char *curr_pos = msg;
    ssize_t read_size = 0;
    ssize_t size = 0;

    do {
        size = recv(fd, curr_pos, hdr.len - (curr_pos - msg), 0);
        read_size += size;
        curr_pos += size;
    } while (read_size < hdr.len && size > 0);

#ifdef DEBUG
    printf("<-(%3u/%3u)-- ", (unsigned) read_size, hdr.len);
    printf("\033[1;32m%s\033[0m\n", msg);
#endif

    return msg;
}

int read_stream(int fd)
{
    char *msg = read_stream_keep(fd);
    int size = strlen(msg);

    free(msg);
    return size;
}
