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
#include <wb_stream.h>
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>
#include <wb_session.h>

#include <stdlib.h>
#include <stdio.h>

static void xmpp_iq_session_join_cb(const char *msg,
                                    enum xmpp_msg_type type,
                                    void *args)
{
    /* Answer:
       <iq from='masterserver@warface/pve_1' type='result'>
         <query xmlns='urn:cryonline:k01'>
           <session_join room_id='4645701477'
                         server='ded8-lv-lw-eu_64013'
                         hostname='xxx.xxx.xxx.xx' port='64013'
                         local='0' session_id='7921418322'/>
           </query>
       </iq>
    */

    if (msg == NULL)
        return;

    char *data = wf_get_query_content(msg);

    if (data != NULL)
    {
        char *ip = get_info(data, "hostname='", "'", NULL);
        int port = get_info_int(data, "port='", "'", NULL);

        printf("Game room started! Leave... (IP/PORT: %s %d)\n", ip, port);

        free(ip);
        free(data);
    }

    if (!session.gameroom.is_safemaster)
        xmpp_iq_gameroom_leave();
    else
        xmpp_iq_gameroom_setplayer(session.gameroom.curr_team, 0,
                                   session.profile.curr_class, NULL, NULL);
}

static void xmpp_iq_gameroom_sync_cb(const char *msg_id,
                                     const char *msg,
                                     void *args)
{
    char *data = wf_get_query_content(msg);
    char *session_node = get_info(data, "<session ", "/>", NULL);

    if (session_node == NULL)
    {
        free(data);
        return;
    }

    int room_status = get_info_int(session_node, "status='", "'", NULL);

    if (room_status == 2)
    {
        t_uid id;

        idh_generate_unique_id(&id);
        idh_register(&id, 0, xmpp_iq_session_join_cb, NULL);

        if (!session.gameroom.joined)
        {
            send_stream_format(session.wfs,
                               "<iq to='masterserver@warface/%s' type='get'>"
                               " <query xmlns='urn:cryonline:k01'>"
                               "  <setcurrentclass current_class='%d'/>"
                               " </query>"
                               "</iq>",
                               session.online.channel,
                               session.profile.curr_class);

            send_stream_format(session.wfs,
                               "<iq id='%s' to='masterserver@warface/%s' type='get'>"
                               " <query xmlns='urn:cryonline:k01'>"
                               "  <session_join/>"
                               " </query>"
                               "</iq>",
                               &id, session.online.channel);
        }

        session.gameroom.joined = 1;

        char *sessionid = get_info(session_node, "id='", "'", NULL);

        if (sessionid != NULL && sessionid[0])
            printf("Session id: %s\n", sessionid);

        free(sessionid);
    }
    else
    {
        if (session.gameroom.joined)
        {
            session.gameroom.joined = 0;
        }
    }

    free(session_node);
    free(data);
}

void xmpp_iq_gameroom_sync_r(void)
{
    qh_register("gameroom_sync", 1, xmpp_iq_gameroom_sync_cb, NULL);
}
