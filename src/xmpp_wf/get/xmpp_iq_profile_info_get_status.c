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
#include <wb_stream.h>
#include <wb_session.h>
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct cb_args
{
    char *nick_to;
    char *jid_to;
};

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

    if (strstr(msg, "type='result'") == NULL)
        return;

    if (!a->nick_to || !a->jid_to)
        return;

    char *info = get_info(msg, "<info", "/>", NULL);

    if (info == NULL)
    {
        xmpp_send_message(session.wfs, session.nickname, session.jid,
                          a->nick_to, a->jid_to,
                          "I don&apos;t know that guy...", NULL);
    }
    else
    {
        char *status = get_info(info, "status='", "'", NULL);
        char *ip = get_info(info, "ip_address='", "'", NULL);

        free(info);

        enum e_status i_status = status ? strtol(status, NULL, 10) : 0;
        const char *s_status = i_status & STATUS_AFK ? "AFK" :
            i_status & STATUS_PLAYING ? "playing" :
            i_status & STATUS_SHOP ? "in shop" :
            i_status & STATUS_INVENTORY ? "in inventory" :
            i_status & STATUS_ROOM ? "in a room" :
            i_status & STATUS_LOBBY ? "in lobby" :
            "offline"; /* wut ? impossible !§§!§ */

        int r = rand() % 3;
        const char *format = r == 0 ? "His IP is %s and is %s" :
            r == 1 ? "That guy ip is %s. He is %s" :
            r == 2 ? "I think %s is his IP. He may be %s" :
            "Stop asking me all these questions !";

        char *message;
        FORMAT(message, format, ip, s_status);

        free(status);
        free(ip);

        xmpp_send_message(session.wfs, session.nickname, session.jid,
                          a->nick_to, a->jid_to,
                          message, NULL);

        free(message);
    }

    free(a->nick_to);
    free(a->jid_to);
    free(a);
}

void xmpp_iq_profile_info_get_status(const char *nickname,
                                     const char *nick_to,
                                     const char *jid_to)
{
    struct cb_args *a = malloc(sizeof (struct cb_args));

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
