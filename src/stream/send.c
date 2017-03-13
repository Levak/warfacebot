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

#include <wb_log.h>
#include <wb_stream.h>
#include <wb_xmpp_wf.h>

#ifdef DEBUG
# include <wb_cvar.h>
# include <wb_querydump.h>
#endif /* DEBUG */

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>

#ifdef __MINGW32__
# include <winsock.h>
#else
# include <sys/socket.h>
#endif

#ifdef USE_TLS
# define SEND(Fd, Buf, Size) tls_send((Fd), (Buf), (Size))
#else
# define SEND(Fd, Buf, Size) send((Fd), (Buf), (Size), MSG_MORE)
#endif

void stream_send_msg(int fd, const char *msg)
{
    static int _protect = -1;

    size_t msg_size = strlen(msg);
    char *compressed = wf_compress_query(msg);
    char *buffer = NULL;

    if (_protect == -1)
        _protect = cvar.online_use_protect;

#ifdef DEBUG
    if (cvar.query_debug)
    {
        if (crypt_is_ready())
            xprintf("%s==(%3u)=> \033[1;31m%s\033[0m",
                    compressed ? "##" : "==", msg_size, msg);
        else
            xprintf("%s--(%3u)-> \033[1;31m%s\033[0m",
                    compressed ? "##" : "--", msg_size, msg);
    }

    querydump_emit("SEND", msg);
#endif /* DEBUG */

    if (compressed != NULL /* && strstr(msg, "to='k01.warface'") == NULL */)
    {
        msg_size = strlen(compressed);
        buffer = compressed;
    }
    else
    {
        buffer = strdup(msg);
        free(compressed);
        compressed = buffer;
    }

    if (_protect)
    {
        struct stream_hdr hdr;

        hdr.magic = STREAM_MAGIC;
        hdr.se = SE_PLAIN;
        hdr.len = msg_size;

        if (crypt_is_ready())
        {
            hdr.se = SE_ENCRYPTED;
            crypt_encrypt((uint8_t *) buffer, msg_size);
        }

        SEND(fd, &hdr, sizeof (hdr));
    }

    SEND(fd, buffer, msg_size);

    free(compressed);
}

void stream_send_ack(int fd)
{
    struct stream_hdr hdr;

    hdr.magic = STREAM_MAGIC;
    hdr.se = SE_CLIENT_ACK;
    hdr.len = 0;

#ifdef DEBUG
    if (cvar.query_debug)
    {
        xprintf("----()-> ACK KEY");
    }
#endif /* DEBUG */

    SEND(fd, &hdr, sizeof (hdr));

    stream_flush(fd);
}

void stream_flush(int fd)
{
    send(fd, "", 0, 0);
}
