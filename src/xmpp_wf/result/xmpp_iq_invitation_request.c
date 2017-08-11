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
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>
#include <wb_session.h>
#include <wb_cvar.h>
#include <wb_log.h>
#include <wb_lang.h>

#include <stdlib.h>
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

    char *data = wf_get_query_content(msg);

    if (!data)
        return;

    char *server = get_info(msg, "from='", "'", "Server");

    char *resource = get_info(data, "ms_resource='", "'", NULL);
    char *ticket = get_info(data, "ticket='", "'", NULL);
    char *group_id = get_info(data, "group_id='", "'", NULL);
    char *room = get_info(data, "room_id='", "'", NULL);
    char *nick_from = get_info(data, " from='", "'", NULL);
    unsigned int is_follow = get_info_int(data, "is_follow='", "'", NULL);

    if (server && resource && ticket && room)
    {
        char accepted = !cvar.wb_safemaster
            && (cvar.wb_accept_room_invitations || is_follow == 1)
            && session.quickplay.pre_uid == NULL
            && session.quickplay.uid == NULL;

        char postponed = cvar.wb_postpone_room_invitations;

        if (is_follow == 0)
        {
            char *s = LANG_FMT(notif_room_invitation, nick_from);
            xprintf("%s (%s)",
                    s,
                    (postponed)
                    ? LANG(notif_postponed)
                    : (accepted)
                    ? LANG(notif_accepted)
                    : LANG(notif_rejected));
            free(s);
        }
        else
        {
            accepted = invitation_complete(nick_from, NULL, INVIT_ACCEPTED, 1);
        }

        if (!postponed)
        {
            /* 1. Confirm or refuse invitation */
            xmpp_send_iq_get(
                JID(server),
                NULL, NULL,
                "<query xmlns='urn:cryonline:k01'>"
                " <invitation_accept ticket='%s' result='%d'/>"
                "</query>",
                ticket,
                accepted ? 0 : 1);

            if (accepted)
            {
                /* 2. Join the room */
                xmpp_iq_gameroom_join(
                    resource,
                    room,
                    group_id,
                    is_follow ? JOIN_FOLLOW : JOIN_INVITE);
            }
        }
    }

    free(nick_from);
    free(server);
    free(ticket);
    free(room);
    free(group_id);
    free(resource);

    free(data);
}

void xmpp_iq_invitation_request_r(void)
{
    qh_register("invitation_request", 1, xmpp_iq_invitation_request_cb, NULL);
}
