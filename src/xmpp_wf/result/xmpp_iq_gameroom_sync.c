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
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>
#include <wb_session.h>

#include <stdlib.h>
#include <stdio.h>

static void xmpp_iq_session_join_cb(const char *msg,
                                          void *args)
{
    char *data = wf_get_query_content(msg);
    char *ip = get_info(data, "hostname='", "'", NULL);
    int port = get_info_int(data, "port='", "'", NULL);

    printf("Game room started! Leave... (IP/PORT: %s %d)\n", ip, port);

    free(ip);
    free(data);

    xmpp_iq_gameroom_leave();
}

static void xmpp_iq_gameroom_sync_cb(const char *msg_id,
                                     const char *msg,
                                     void *args)
{
    char *data = wf_get_query_content(msg);
    int room_status = get_info_int(data, "status='", "'", NULL);

    if (room_status == 2)
    {
        t_uid id;

        idh_generate_unique_id(&id);
        idh_register(&id, 0, xmpp_iq_session_join_cb, NULL);

        send_stream_format(session.wfs,
                           "<iq id='%s' to='masterserver@warface/%s' type='get'>"
                           " <query xmlns='urn:cryonline:k01'>"
                           "  <session_join/>"
                           " </query>"
                           "</iq>",
                           &id, session.channel);

        char *sessionid = get_info(data, "session id='", "'", NULL);

        if (sessionid != NULL && sessionid[0])
            printf("Session id: %s\n", sessionid);

        free(sessionid);
    }

    free(data);
}

void xmpp_iq_gameroom_sync_r(void)
{
    qh_register("gameroom_sync", 1, xmpp_iq_gameroom_sync_cb, NULL);
}
