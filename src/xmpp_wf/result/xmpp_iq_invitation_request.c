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

static void xmpp_iq_invitation_request_cb(const char *msg_id, const char *msg)
{
    /* Accept any invitation
       <iq from='masterserver@warface/pve_12' id='uid0002d87c' type='get'>
        <query xmlns='urn:cryonline:k01'>
         <data query_name='invitation_request' from='XXX'
             ticket='XXXX_XXXX_XXXX' room_id='2416'
             ms_resource='pve_12' is_follow='0'
             group_id='be4ab6d9-b03a-4c2f-bd64-d8acc7e7d319'
             compressedData='...' originalSize='2082'/>
        </query>
       </iq>
     */

    char *server = get_info(msg, "from='", "'", "Server");
    char *data = wf_get_query_content(msg);

    if (!data)
        return;

    char *resource = get_info(data, "ms_resource='", "'", "Resource");
    char *ticket = get_info(data, "ticket='", "'", "Ticket");
    char *room = get_info(data, "room_id='", "'", "Room ID");
    char *group = get_info(data, "group_id='", "'", "Group ID");

    if (server && resource && ticket && room && group)
    {
        /* 1. Change channel if invitation was not on the same server */
        if (strcmp(session.channel, resource))
            xmpp_iq_join_channel(resource);

        /* 2. Confirm invitation */
        send_stream_format(session.wfs,
                           "<iq to='%s' type='get'>"
                           " <query xmlns='urn:cryonline:k01'>"
                           "  <invitation_accept ticket='%s' result='0'/>"
                           " </query>"
                           "</iq>",
                           server, ticket);

        /* 3. Join the room */
        send_stream_format(session.wfs,
                           "<iq to='%s' type='get'>"
                           " <query xmlns='urn:cryonline:k01'>"
                           "  <gameroom_join room_id='%s' team_id='0' group_id='%s'"
                           "     status='1' class_id='1' join_reason='0'/>"
                           " </query>"
                           "</iq>",
                           server, room, group);

        /* 4. Join XMPP room */
        FORMAT(session.room_jid, "room.%s.%s@conference.warface", resource, room);
        send_stream_format(session.wfs,
                           "<presence to='%s/%s'/>",
                           session.room_jid, session.nickname);


        /* 5. Change public status */
        xmpp_iq_player_status(STATUS_ONLINE | STATUS_ROOM);

        free(server);
        free(ticket);
        free(room);
        free(resource);

        free(session.group_id);
        session.group_id = group;
    }

    free(data);
}

void xmpp_iq_invitation_request_r(void)
{
    qh_register("invitation_request", xmpp_iq_invitation_request_cb);
}
