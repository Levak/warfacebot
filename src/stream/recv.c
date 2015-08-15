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

#include <errno.h>

char *read_stream_keep(int fd)
{
    struct stream_hdr hdr;
    char *hdr_pos = (char *) &hdr;
    size_t hdr_read = 0;

    do {
        ssize_t size = recv(fd, hdr_pos, sizeof(hdr) - (hdr_pos - (char *) &hdr), 0);

        if (size <= 0)
        {
            perror("read");
            return NULL;
        }
        hdr_read += size;
        hdr_pos += size;
    }
    while (hdr_read < sizeof (hdr));

    if (hdr.magic != 0xFEEDDEAD)
        return NULL;

    if (hdr.len == 0)
        return NULL;

    char *msg = calloc(hdr.len + 1, 1);
    char *curr_pos = msg;
    ssize_t read_size = 0;

    do {
        ssize_t size = recv(fd, curr_pos, hdr.len - (curr_pos - msg), 0);

        if (size <= 0)
        {
            free(msg);
            return NULL;
        }

        read_size += size;
        curr_pos += size;
    } while (read_size < hdr.len);

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
