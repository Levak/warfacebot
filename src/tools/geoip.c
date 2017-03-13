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

#include <wb_tools.h>
#include <wb_geoip.h>

#include <wb_log.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#ifdef __MINGW32__
# include <winsock.h>
#else
# include <sys/socket.h>
# include <netinet/in.h>
# include <netdb.h>
#endif

struct geoip *geoip_get_info(const char *ip, int full)
{
    struct geoip *g = NULL;

    int fd = -1;
    {
        fd = socket(AF_INET, SOCK_STREAM, 0);

        struct sockaddr_in serv_addr;
        struct hostent *server;

        server = gethostbyname("freegeoip.net");
        if (server == NULL)
        {
            eprintf("ERROR gethostbyname %s\n", strerror(errno));
            close(fd);

            return NULL;
        }

        memset((char *) &serv_addr, 0, sizeof(serv_addr));
        memcpy((char *) &serv_addr.sin_addr.s_addr,
               (char *) server->h_addr,
               server->h_length);

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(80);

        if (connect(fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        {
            eprintf("ERROR connect %s\n", strerror(errno));
            close(fd);

            return NULL;
        }
    }

    char *csv = NULL;
    {
        char *request;
        FORMAT(request, "GET /csv/%s HTTP/1.1\nHost: freegeoip.net\n\n", ip);

        send(fd, request, strlen(request), 0);
        free(request);

        char buff[4096];
        ssize_t len = recv(fd, buff, sizeof(buff), 0);

        if (len > 0)
        {
            if (len >= (ssize_t) sizeof(buff))
                len = sizeof(buff);

            buff[len - 1] = 0;

            csv = get_info(buff, "\r\n\r\n", "\r", NULL);
        }
    }

    if (csv != NULL)
    {
        char *saveptr = NULL;
        char *sep = ",";

        g = calloc(1, sizeof (struct geoip));

        g->ip = get_token(csv, sep, &saveptr);
        g->country_code = get_token(NULL, sep, &saveptr);
        g->country_name = get_token(NULL, sep, &saveptr);

        if (full)
        {
            g->region_code = get_token(NULL, sep, &saveptr);
            g->region_name = get_token(NULL, sep, &saveptr);
            g->city = get_token(NULL, sep, &saveptr);
            g->zip_code = get_token(NULL, sep, &saveptr);
            g->time_zone = get_token(NULL, sep, &saveptr);
            g->latitude = get_token(NULL, sep, &saveptr);
            g->longitude = get_token(NULL, sep, &saveptr);
            g->metro_code = get_token(NULL, sep, &saveptr);
        }
    }

    free(csv);
    close(fd);

    return g;
}

void geoip_free(struct geoip *g)
{
    if (g != NULL)
    {
        free(g->ip);
        free(g->country_code);
        free(g->country_name);
        free(g->region_code);
        free(g->region_name);
        free(g->city);
        free(g->zip_code);
        free(g->time_zone);
        free(g->latitude);
        free(g->longitude);
        free(g->metro_code);
        free(g->isp);
    }

    free(g);
}
