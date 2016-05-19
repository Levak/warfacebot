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

#include <wb_tools.h>
#include <wb_stream.h>
#include <wb_session.h>
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>


static void xmpp_iq_peer_status_update_cb ( const char *msg,
enum xmpp_msg_type type,
	void *args )
{
	const struct friend *f = ( const struct friend * ) args;

	if ( type & XMPP_TYPE_ERROR )
	{
		//printf ( "STATUS WAS %d\n", f->status );
		//if ( msg )
		//{
		//	puts ( msg );
		//	char *data = wf_get_query_content ( msg );
		//	puts ( data );
		//}
		int status = STATUS_OFFLINE;
		char *nick = strdup ( f->nickname );
		char *pid = strdup ( f->profile_id );
		unsigned int exp = f->experience;
		char *place_token = strdup ( f->place_token );
		char *place_info_token = strdup ( f->place_info_token );
		char *mode_info_token = strdup ( f->mode_info_token );
		char *mission_info_token = strdup ( f->mission_info_token );

		friend_list_update ( NULL, nick, pid, status, exp, place_token, place_info_token, mode_info_token, mission_info_token );

#ifdef DBUS_API
		dbus_api_emit_status_update ( nick, status, exp, 0 );
#endif /* DBUS_API */

		free ( nick );
		free ( pid );
		free ( place_token );
		free ( place_info_token );
		free ( mode_info_token );
		free ( mission_info_token );
	}
}

/*
Type: f_list_callback
*/
void xmpp_iq_peer_status_update_friend ( const struct friend *f, void *args )
{
	if ( f->jid )
		xmpp_iq_peer_status_update ( f );
}

void xmpp_iq_peer_status_update ( const struct friend *f )
{
	if ( f == NULL || f->jid == NULL )
		return;

	t_uid id;

	idh_generate_unique_id ( &id );
	idh_register ( &id, 0, xmpp_iq_peer_status_update_cb, (void *) f );

	/* Inform to our friends our status */
	send_stream_format ( session.wfs,
						 "<iq id='%s' to='%s' type='get'>"
						 " <query xmlns='urn:cryonline:k01'>"
						 "  <peer_status_update nickname='%s' profile_id='%s'"
						 "     status='%u' experience='%u'"
						 "     place_token='' place_info_token=''/>"
						 " </query>"
						 "</iq>",
						 &id, f->jid,
						 session.profile.nickname, session.profile.id,
						 session.profile.status, session.profile.experience );
}