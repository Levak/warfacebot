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

#include <stdlib.h>

static void xmpp_iq_follow_send_cb(const char *msg_id,
                                   const char *msg,
                                   void *args)
{
    /* Answer:
       <iq from='xxxxx@warface/GameClient' id='uid000002c1' type='get'>
        <query xmlns='urn:cryonline:k01'>
         <follow_send nickname='xxxxxx' profile_id='xxxx'/>
        </query>
       </iq>
    */

    if (msg != NULL)
    {
        char *from_jid = get_info(msg, "from='", "'", NULL);
        char *nickname = get_info(msg, "nickname='", "'", NULL);

        if (from_jid != NULL && nickname != NULL)
        {
            /* Accept any follow request */
            xmpp_iq_invitation_send(nickname, 1, NULL, NULL);

            xmpp_send_iq_result(
                JID(from_jid),
                msg_id,
                "<query xmlns='urn:cryonline:k01'>"
                " <follow_send/>"
                "</query>",
                NULL);
        }

        free(nickname);
        free(from_jid);
    }
}

void xmpp_iq_follow_send_r(void)
{
    qh_register("follow_send", 1, xmpp_iq_follow_send_cb, NULL);
}
