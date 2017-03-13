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

#include <wb_xmpp.h>
#include <wb_session.h>
#include <wb_xmpp_wf.h>
#include <wb_list.h>
#include <wb_dbus.h>

void xmpp_iq_player_status(int status)
{
    int in_a_room = session.gameroom.jid != NULL;

    xmpp_send_iq_get(
        JID_K01,
        NULL, NULL,
        "<query xmlns='urn:cryonline:k01'>"
        "<player_status prev_status='%u' new_status='%u' to='%s'/>"
        "</query>",
        session.online.status,
        status,
        in_a_room ? session.online.channel : "");
}
