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

#include <wb_tools.h>
#include <wb_geoip.h>
#include <wb_stream.h>
#include <wb_session.h>
#include <wb_xml.h>
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>
#include <wb_cmd.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

struct cb_args
{
    f_cmd_whois_cb cb;
    void *args;
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

    if (a->cb)
        a->cb(a->ip,
              g != NULL ? g->country_name : NULL,
              s_status,
              a->args);

    geoip_free(g);

    free(a->ip);
    free(a);

    pthread_exit(NULL);
}

static void cmd_whois_cb(const char *info, void *args)
{
    struct cb_args *a = (struct cb_args *) args;

    if (info == NULL)
    {
        if (a->cb)
            a->cb(NULL, NULL, NULL, a->args);

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

void *cmd_whisper_args(const char *nick_to, const char *jid_to)
{
    struct whisper_cb_args *a = calloc(1, sizeof (struct whisper_cb_args));

    if (nick_to != NULL)
        a->nick_to = strdup(nick_to);
    if (jid_to != NULL)
        a->jid_to = strdup(jid_to);

    return (void *) a;
}

void cmd_whois(const char *nickname,
               f_cmd_whois_cb cb,
               void *args)
{
    struct cb_args *a = calloc(1, sizeof (struct cb_args));

    a->cb = cb;
    a->args = args;

    xmpp_iq_profile_info_get_status(nickname, cmd_whois_cb, a);
}

void cmd_whois_console_cb(const char *ip, const char *country, const char *status, void *args)
{
    if (ip == NULL || status == NULL)
        LOGPRINT("%s", "No such user connected\n");
    else if (country == NULL)
        LOGPRINT("ip:%s is %s\n", ip, status);
    else
        LOGPRINT("ip:%s (%s) is %s\n", ip, country, status);
}

void cmd_whois_whisper_cb(const char *ip, const char *country, const char *status, void *args)
{
    struct whisper_cb_args *a = (struct whisper_cb_args *) args;

    if (ip == NULL)
    {
       xmpp_send_message(a->nick_to, a->jid_to,
                              "I don't know that guy...");
    }
    else
    {
        char *message;

        if (country == NULL)
        {
            FORMAT(message, "He's %s", status);
        }
        else
        {
            int r = time(NULL) % 3;
            const char *fmt = r == 0 ? "He's from %s... currently %s" :
                r == 1 ? "That's a guy from %s. He is %s" :
                "I met him in %s but now he's %s";

            FORMAT(message, fmt, country, status);
        }

        xmpp_send_message(a->nick_to, a->jid_to, message);
        free(message);
    }

    free(a->nick_to);
    free(a->jid_to);
    free(a);
}
