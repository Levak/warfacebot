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
#include <wb_session.h>
#include <wb_xmpp.h>

#include <stdlib.h>

static void xmpp_iq_p2p_ping_cb(const char *msg_id,
                                const char *msg,
                                void *args)
{
    /* Reply to peer to peer pings
       <iq from='xxxxx@warface/GameClient' id='uid0002d87c' type='get'>
        <query xmlns='urn:cryonline:k01'>
         <p2p_ping/>
        </query>
       </iq>
     */

    char *from = get_info(msg, "from='", "'", NULL);

    xmpp_send_iq_result(
        JID(from),
        msg_id,
        "<query xmlns='urn:cryonline:k01'>"
        " <p2p_ping/>"
        "</query>",
        NULL);

    free(from);
}

void xmpp_iq_p2p_ping_r(void)
{
    qh_register("p2p_ping", 1, xmpp_iq_p2p_ping_cb, NULL);
}
