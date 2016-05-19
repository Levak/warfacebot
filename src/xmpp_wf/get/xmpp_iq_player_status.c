/**
 * WarfaceBot, a blind XMPP client for Warface (FPS)
 * Copyright (C) 2015, 2016 Levak Borok <levak92@gmail.com>
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

#include <wb_stream.h>
#include <wb_session.h>
#include <wb_xmpp_wf.h>
#include <wb_list.h>
#include <wb_dbus.h>

void xmpp_iq_player_status ( int status )
{
	send_stream_format ( session.wfs,
						 "<iq to='k01.warface' type='get'>"
						 "<query xmlns='urn:cryonline:k01'>"
						 "<player_status prev_status='%u' new_status='%u'"
						 "               to='%s'/>"
						 "</query>"
						 "</iq>",
						 session.profile.status, status,
						 session.gameroom.joined ? session.online.channel : "" );

	session.profile.status = status;

	list_foreach ( session.profile.friends,
				   (f_list_callback) xmpp_iq_peer_status_update_friend,
				   NULL );

	list_foreach ( session.clan.clanmates,
				   (f_list_callback) xmpp_iq_peer_clan_member_update_clanmate,
				   NULL );
#ifdef DBUS_API
	dbus_api_emit_status_update ( session.profile.nickname,
								  session.profile.status,
								  session.profile.experience,
								  session.clan.points );
#endif
}