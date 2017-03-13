/**
 * WarfaceBot, a blind XMPP client for Warface (FPS)
 * Copyright (C) 2015-2017 Levak Borok <levak92@gmail.com>
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

#ifdef __MINGW32__
# include <winsock.h>
#else
# include <sys/socket.h>
#endif

#include <wb_stream.h>
#include <wb_cvar.h>
#include <wb_log.h>

#ifdef DEBUG
# include <wb_cvar.h>
# include <wb_querydump.h>
#endif /* DEBUG */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef USE_TLS
# define RECV(Fd, Buf, Size) tls_recv((Fd), (Buf), (Size))
# define PERROR(Str, Ret) tls_perror((Str), (Ret))
#else
# define RECV(Fd, Buf, Size) recv((Fd), (Buf), (Size), 0)
# define PERROR(Str, Ret) perror((Str))
#endif

char *stream_read(int fd)
{
    static int _protect = -1;
    struct stream_hdr hdr = { 0 };
    ssize_t read_size = 0;
    uint8_t *msg = NULL;
    uint8_t *curr_pos = NULL;

    if (_protect == -1)
        _protect = cvar.online_use_protect;

    if (_protect)
    {
        uint8_t *hdr_pos = (uint8_t *) &hdr;
        size_t hdr_read = 0;

        do {
            ssize_t size = RECV(fd, hdr_pos, sizeof(hdr) - (hdr_pos - (uint8_t *) &hdr));

            if (size <= 0)
            {
                PERROR("read", size);
                return NULL;
            }

            hdr_read += size;
            hdr_pos += size;
        }
        while (hdr_read < sizeof (hdr));

        if (hdr.magic != STREAM_MAGIC)
        {
            eprintf("Bad header: %x", hdr.magic);
            return NULL;
        }

        if (hdr.len == 0)
            return NULL;

        msg = calloc(hdr.len + 1, 1);
        curr_pos = msg;

        do {
            ssize_t size = RECV(fd, curr_pos, hdr.len - (curr_pos - msg));

            if (size <= 0)
            {
                free(msg);
                return NULL;
            }

            read_size += size;
            curr_pos += size;
        } while (read_size < hdr.len);
    }
    else
    {
        ssize_t buff_size = 256;
        msg = calloc(buff_size, 1);
        curr_pos = msg;

        do {
            ssize_t size = RECV(fd, curr_pos, buff_size/2);

            if (size <= 0)
            {
                free(msg);
                return NULL;
            }

            read_size += size;
            curr_pos += size;

            if (curr_pos[-1] != '>' && curr_pos[-1] != '\0')
            {
                uint8_t *old_msg = msg;
                buff_size = buff_size * 2;
                msg = realloc(msg, buff_size);
                curr_pos = (curr_pos - old_msg) + msg;
                memset(curr_pos, 0, buff_size - (curr_pos - msg));
            }

        } while (curr_pos[-1] != '>' && curr_pos[-1] != '\0');
    }

    switch (hdr.se)
    {
        case SE_PLAIN:
        {
#ifdef DEBUG
            if (cvar.query_debug)
            {
                xprintf("<-(%3u/%3u)-- \033[1;32m%s\033[0m",
                        (unsigned) read_size, hdr.len, msg);
            }
#endif /* DEBUG */
            break;
        }

        case SE_ENCRYPTED:
        {
            crypt_decrypt(msg, hdr.len);
#ifdef DEBUG
            if (cvar.query_debug)
            {
                xprintf("<-(%3u/%3u)== \033[1;32m%s\033[0m",
                        (unsigned) read_size, hdr.len, msg);
            }
#endif /* DEBUG */
            break;
        }

        case SE_SERVER_KEY:
        {
            if (cvar.online_use_protect)
            {
                char *end = (char *) msg + 3;
                int key = strtol((char *) msg, &end, 10);
#ifdef DEBUG
                if (cvar.query_debug)
                {
                    xprintf("<-(%3u/%3u) KEY: %d",
                            (unsigned) read_size, hdr.len, key);
                }
#endif /* DEBUG */
                crypt_init(key);
                stream_send_ack(fd);
            }

            free(msg);

            return stream_read(fd);
        }

        default:
#ifdef DEBUG
            eprintf("Unsupported stream crypt method: %d", hdr.se);
#endif /* DEBUG */
            break;
    };

#ifdef DEBUG
    querydump_emit("RECV", (char *) msg);
#endif /* DEBUG */

    return (char *) msg;
}
