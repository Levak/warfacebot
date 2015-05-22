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

#include <wb_stream.h>
#include <wb_session.h>
#include <wb_xmpp_wf.h>
#include <wb_list.h>

void xmpp_iq_player_status(int status)
{
    send_stream_format(session.wfs,
                       "<iq to='k01.warface' type='get'>"
                       "<query xmlns='urn:cryonline:k01'>"
                       "<player_status prev_status='%d' new_status='%d'"
                       "               to='%s'/>"
                       "</query>"
                       "</iq>",
                       session.status, status, "");
    session.status = status;

    list_foreach(session.friends,
                 (f_list_callback) xmpp_iq_peer_status_update_friend,
                 NULL);
}
