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

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef __MINGW32__
# include <winsock.h>
#else
# include <sys/socket.h>
# include <netinet/in.h>
# include <netdb.h>
# include <unistd.h>
#endif

int stream_connect(const char *hostname, int port)
{
#ifdef __MINGW32__
    WORD wVersionRequested;
    WSADATA wsaData;

    wVersionRequested = MAKEWORD(2, 2);
    WSAStartup(wVersionRequested, &wsaData);
#endif

    int wfs = socket(AF_INET, SOCK_STREAM, 0);

    if (wfs < 0)
    {
        eprintf("ERROR socket");

        return -1;
    }

    struct sockaddr_in serv_addr;
    struct hostent *server;

    server = gethostbyname(hostname);

    if (server == NULL)
    {
        eprintf("ERROR gethostbyname");
        close(wfs);

        return -1;
    }

    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    memcpy((char *) &serv_addr.sin_addr.s_addr,
          (char *) server->h_addr,
          server->h_length);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (connect(wfs, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        eprintf("ERROR connect");
        eprintf("%s", strerror(errno));
        close(wfs);

        return -1;
    }

    return wfs;
}
