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

static void xmpp_iq_ping_cb(const char *msg_id, const char *msg, void *args)
{
    /* Anwser to pings
       <iq from='warface' id='3160085767' to='XXXX@warface/GameClient' type='get'>
        <ping xmlns='urn:xmpp:ping'/>
       </iq>
    */

    xmpp_send_iq_result(
        JID_HOST,
        msg_id,
        NULL,
        NULL);
}

void xmpp_iq_ping_r(void)
{
    qh_register("ping", 1, xmpp_iq_ping_cb, NULL);
}
