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

#include <wb_tools.h>
#include <wb_geoip.h>
#include <wb_stream.h>
#include <wb_session.h>
#include <wb_xml.h>
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

struct cb_args
{
    char *nick_to;
    char *jid_to;
    char *ip;
    enum e_status status;
};

static void *thread_get_geoloc(void *vargs)
{
    struct cb_args *a = (struct cb_args *) vargs;
    struct geoip *g = geoip_get_info(a->ip, 0);

    enum e_status i_status = a->status;
    const char *s_status = a->status & STATUS_AFK ? "AFK" :
        i_status & STATUS_PLAYING ? "playing" :
        i_status & STATUS_SHOP ? "in shop" :
        i_status & STATUS_INVENTORY ? "in inventory" :
        i_status & STATUS_ROOM ? "in a room" :
        i_status & STATUS_LOBBY ? "in lobby" :
        i_status & STATUS_ONLINE ? "connecting" :
        "offline"; /* wut ? impossible !§§!§ */

    if (a->nick_to == NULL || a->jid_to == NULL)
    {
        if (g == NULL)
            printf("ip:%s is %s\n", a->ip, s_status);
        else
        {
            printf("ip:%s (%s) is %s\n", a->ip, g->country_name, s_status);
            geoip_free(g);
        }
    }
    else
    {
        int r = time(NULL) % 3;
        const char *format = r == 0 ? "He's from %s... currently %s" :
            r == 1 ? "That's a guy from %s. He is %s" :
            "I met him in %s but now he's %s";

        char *message;

        if (g == NULL)
            FORMAT(message, "He's %s", s_status);
        else
        {
            FORMAT(message, format, g->country_name, s_status);
            geoip_free(g);
        }

        xmpp_send_message(a->nick_to, a->jid_to, message);

        free(message);
    }

    free(a->ip);
    free(a->nick_to);
    free(a->jid_to);
    free(a);

    pthread_exit(NULL);
}

static void cmd_whois_cb(const char *info, void *args)
{
    struct cb_args *a = (struct cb_args *) args;

    if (info == NULL)
    {
        if (a->nick_to == NULL || a->jid_to == NULL)
            printf("No such user connected\n");
        else
            xmpp_send_message(a->nick_to, a->jid_to,
                              "I don't know that guy...");
        free(a->nick_to);
        free(a->jid_to);
        free(a);
    }
    else
    {
        a->status = get_info_int(info, "status='", "'", NULL);
        a->ip = get_info(info, "ip_address='", "'", NULL);

        pthread_t thread_gl;

        if (pthread_create(&thread_gl, NULL, thread_get_geoloc, args) == -1)
            perror("pthread_create");

        pthread_detach(thread_gl);
    }
}

void cmd_whois(const char *nickname,
               const char *nick_to,
               const char *jid_to)
{
    struct cb_args *a = calloc(1, sizeof (struct cb_args));

    if (a->nick_to)
        a->nick_to = strdup(nick_to);
    if (a->jid_to)
        a->jid_to = strdup(jid_to);

    xmpp_iq_profile_info_get_status(nickname, cmd_whois_cb, a);
}
