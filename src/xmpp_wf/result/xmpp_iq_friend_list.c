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

static void xmpp_iq_friend_list_cb(const char *msg_id, const char *msg)
{
    /* Record firends to list
       <iq from='masterserver@warface/pve_12' type='get'>
        <query xmlns='urn:cryonline:k01'>
         <friend_list>
          <friend jid='XXX' profile_id='XXX' nickname='XXX'
               status='XXX' experience='XXX' location='XXX'/>
         </friend_list>
        </query>
       </iq>
    */

    char *data = wf_get_query_content(msg);

    /* TODO: For entire friendlist */
    char *jid = get_info(data, "jid='", "'", "FRIEND JID");

#if 0
    printf("\n\nDECODED:\n%s\n\n", data);
#endif

    if (jid && *jid)
    {
        session.friend = jid;
        xmpp_iq_peer_status_update(jid);
    }
    else
        free(jid);

    free(data);
}

void xmpp_iq_friend_list_r(void)
{
    qh_register("friend_list", xmpp_iq_friend_list_cb);
}
