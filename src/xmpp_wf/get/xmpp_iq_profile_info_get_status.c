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
    char *status;
};

static void *thread_get_geoloc(void *vargs);

static void xmpp_iq_profile_info_get_status_cb(const char *msg, void *args)
{
    /* Answer:
       <iq from='k01.warface' type='result'>
        <query xmlns='urn:cryonline:k01'>
         <profile_info_get_status nickname='xxxx'>
          <profile_info>
           <info nickname='xxxx' online_id='xxxx@warface/GameClient'
                 status='33' profile_id='xxx' user_id='xxxxx'
                 rank='xx' tags='' ip_address='xxx.xxx.xxx.xxx'
                 login_time='xxxxxxxxxxx'/>
          </profile_info>
         </profile_info_get_status>
        </query>
       </iq>
     */

    struct cb_args *a = (struct cb_args *) args;

    if (strstr(msg, "type='result'") == NULL
        || a->nick_to == NULL
        || a->jid_to == NULL)
    {
        free(a->nick_to);
        free(a->jid_to);
        free(a);
    }

    char *info = get_info(msg, "<info", "/>", NULL);

    if (info == NULL)
    {
        xmpp_send_message(session.wfs, session.nickname, session.jid,
                          a->nick_to, a->jid_to,
                          "I don&apos;t know that guy...", NULL);
    }
    else
    {
        a->status = get_info(info, "status='", "'", NULL);
        a->ip = get_info(info, "ip_address='", "'", NULL);

        pthread_t thread_gl;

        if (pthread_create(&thread_gl, NULL, thread_get_geoloc, args) == -1)
            perror("pthread_create");

        pthread_detach(thread_gl);
    }

    free(info);
}

static void *thread_get_geoloc(void *vargs)
{
    struct cb_args *a = (struct cb_args *) vargs;
    struct geoip *g = geoip_get_info(a->ip, 0);

    enum e_status i_status = a->status ? strtol(a->status, NULL, 10) : 0;
    const char *s_status = i_status & STATUS_AFK ? "AFK" :
        i_status & STATUS_PLAYING ? "playing" :
        i_status & STATUS_SHOP ? "in shop" :
        i_status & STATUS_INVENTORY ? "in inventory" :
        i_status & STATUS_ROOM ? "in a room" :
        i_status & STATUS_LOBBY ? "in lobby" :
        i_status & STATUS_ONLINE ? "connecting" :
        "offline"; /* wut ? impossible !§§!§ */

    int r = time(NULL) % 3;
    const char *format = r == 0 ? "He&apos;s from %s... currently %s" :
        r == 1 ? "That&apos;s a guy from %s. He is %s" :
        "I met him in %s but now he&apos;s %s";

    char *message;
    FORMAT(message, format, g->country_name, s_status);

    geoip_free(g);

    xmpp_send_message(session.wfs, session.nickname, session.jid,
                      a->nick_to, a->jid_to,
                      message, NULL);

    free(message);

    free(a->status);
    free(a->ip);
    free(a->nick_to);
    free(a->jid_to);
    free(a);

    pthread_exit(NULL);
}

void xmpp_iq_profile_info_get_status(const char *nickname,
                                     const char *nick_to,
                                     const char *jid_to)
{
    struct cb_args *a = calloc(1, sizeof (struct cb_args));

    a->nick_to = strdup(nick_to);
    a->jid_to = strdup(jid_to);

    t_uid id;

    idh_generate_unique_id(&id);
    idh_register(&id, 0, xmpp_iq_profile_info_get_status_cb, a);

    send_stream_format(session.wfs,
                       "<iq to='k01.warface' type='get' id='%s'>"
                       "<query xmlns='urn:cryonline:k01'>"
                       "<profile_info_get_status nickname='%s'/>"
                       "</query>"
                       "</iq>",
                       &id, nickname);
}
