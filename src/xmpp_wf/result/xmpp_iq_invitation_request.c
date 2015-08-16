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
#include <string.h>

static void xmpp_iq_invitation_request_cb(const char *msg_id,
                                          const char *msg,
                                          void *args)
{
    /* Accept any invitation
       <iq from='masterserver@warface/pve_12' id='uid0002d87c' type='get'>
        <query xmlns='urn:cryonline:k01'>
         <data query_name='invitation_request' from='XXX'
             ticket='XXXX_XXXX_XXXX' room_id='2416'
             ms_resource='pve_12' is_follow='0'
             compressedData='...' originalSize='2082'/>
        </query>
       </iq>
     */

    char *server = get_info(msg, "from='", "'", "Server");
    char *data = wf_get_query_content(msg);

    if (!data)
        return;

    char *resource = get_info(data, "ms_resource='", "'", NULL);
    char *ticket = get_info(data, "ticket='", "'", NULL);
    char *room = get_info(data, "room_id='", "'", NULL);
    char *nick_from = get_info(data, " from='", "'", NULL);

    printf("Invitation from %s\n", nick_from);

    if (server && resource && ticket && room)
    {
        /* 1. Confirm invitation */
        send_stream_format(session.wfs,
                           "<iq to='%s' type='get'>"
                           " <query xmlns='urn:cryonline:k01'>"
                           "  <invitation_accept ticket='%s' result='0'/>"
                           " </query>"
                           "</iq>",
                           server, ticket);

        /* 2. Join the room */
        xmpp_iq_gameroom_join(resource, room);
    }

    free(nick_from);
    free(server);
    free(ticket);
    free(room);
    free(resource);

    free(data);
}

void xmpp_iq_invitation_request_r(void)
{
    qh_register("invitation_request", 1, xmpp_iq_invitation_request_cb, NULL);
}
