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

static void xmpp_iq_peer_status_update_cb(const char *msg_id, const char *msg)
{
    /* Answer
       <iq from='xxxxxx@warface/GameClient' to='xxxxx@warface/GameClient' type='get'>
        <query xmlns='urn:cryonline:k01'>
         <peer_status_update nickname='xxxx' profile_id='xxxx' status='13'
                             experience='xxxx' place_token='@ui_playerinfo_inlobby'
                             place_info_token=''/>
        </query>
       </iq>
    */

    char *jid = get_info(msg, "from='", "'", "FRIEND JID");

    if (!list_contains(session.friends, jid))
        list_add(session.friends, jid);
    else
        free(jid);
}

void xmpp_iq_peer_status_update_r(void)
{
    qh_register("peer_status_update", xmpp_iq_peer_status_update_cb);
}
