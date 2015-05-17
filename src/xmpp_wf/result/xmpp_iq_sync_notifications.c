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
#include <wb_game_version.h>

#include <stdio.h>
#include <stdlib.h>

enum e_notif_type
{
    NOTIF_ACHIEVEMENT = 4,
    NOTIF_FRIEND_REQUEST = 64,
    NOTIF_ANNOUNCEMENT = 512,
};

void xmpp_iq_sync_notifications_cb(const char *msg_id, const char *msg)
{
    /* Answer:
       <iq from='masterserver@warface/pve_11' type='get'>
        <query xmlns='urn:cryonline:k01'>
         <sync_notifications>
          <notif id='77644810' type='64' confirmation='1'
                 from_jid='masterserver@warface/pve_11'>
           <invitation initiator='xxxx' target='yyyy'/>
          </notif>
         </sync_notifications>
         </query>
       </iq>
    */

    char *query = get_info(msg, ">", "</iq>", NULL);
    char *from_jid = get_info(msg, "from_jid='", "'", NULL);
    char *notif_id = get_info(query, "id='", "'", NULL);
    char *notif_type = get_info(query, "type='", "'", NULL);

    if (notif_type == NULL)
    {
        fprintf(stderr, "Cannot determine notification type: \n%s\n", query);
        return;
    }

    enum e_notif_type i_notif_type = strtol(notif_type, NULL, 10);

    switch (i_notif_type)
    {
        /* Accept any friend requests */
        case NOTIF_FRIEND_REQUEST:
            send_stream_format(session.wfs,
                               "<iq to='masterserver@warface/%s' type='get'>"
                               " <query xmlns='urn:cryonline:k01'>"
                               "  <confirm_notification>"
                               "   <notif id='%s' type='%s'>"
                               "    <confirmation result='0' status='%d' location=''/>"
                               "   </notif>"
                               "  </confirm_notification>"
                               " </query>"
                               "</iq>",
                               session.channel, notif_id, notif_type, session.status);
            break;
        default:
            break;
    }

    free(query);
    free(from_jid);
    free(notif_type);
    free(notif_id);
}

void xmpp_iq_sync_notifications_r(void)
{
    qh_register("sync_notifications", xmpp_iq_sync_notifications_cb);
}
