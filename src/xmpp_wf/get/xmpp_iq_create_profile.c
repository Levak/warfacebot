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
#include <wb_game.h>
#include <wb_mission.h>
#include <wb_dbus.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void xmpp_iq_create_profile_cb ( const char *msg,
enum xmpp_msg_type type,
	void *args )
{
	char *data = wf_get_query_content ( msg );

	if ( type & XMPP_TYPE_ERROR )
	{
		fprintf ( stderr, "Failed to create profile\nReason: " );

		int code = get_info_int ( msg, "code='", "'", NULL );
		int custom_code = get_info_int ( msg, "custom_code='", "'", NULL );

		switch ( code )
		{
			case 503:
				fprintf ( stderr, "Invalid channel (%s)\n", session.channel );
				return;
			case 8:
				switch ( custom_code )
				{
					case 0:
						fprintf ( stderr, "Invalid token (%s) or userid (%s)\n",
								  session.active_token,
								  session.online_id );
						return;
					case 1:
						fprintf ( stderr, "Invalid profile_id (%s)\n",
								  session.profile_id );
						return;
					case 2:
						fprintf ( stderr, "Game version mismatch (%s)\n",
								  game_version_get ( ) );
						return;
					case 3:
						fprintf ( stderr, "Banned\n" );
						break;
					default:
						fprintf ( stderr, "Unknown\n" );
						return;
				}
			default:
				fprintf ( stderr, "Unknown\n" );
				return;
		}

		return;
	}

	session.profile_id = get_info ( data, "profile_id='", "'", "PROFILE ID" );
	session.nickname = get_info ( data, "nick='", "'", "NICKNAME" );

#ifdef DBUS_API
	dbus_api_setup ( );
#endif

	/* Ask for today's missions list */
	mission_list_update ( NULL, NULL );

	free ( data );
}

void xmpp_iq_create_profile ( void )
{
	t_uid id;

	idh_generate_unique_id ( &id );
	idh_register ( &id, 0, xmpp_iq_create_profile_cb, NULL );

	send_stream_format ( session.wfs,
						 "<iq id='%s' to='k01.warface' type='get'>"
						 "<query xmlns='urn:cryonline:k01'>"
						 "<create_profile version='%s' region_id='global'"
						 "                user_id='%s' token='%s'"
						 "                nickname='' resource='%s'/>"
						 "</query>"
						 "</iq>",
						 &id, game_version_get ( ),
						 session.online_id, session.active_token,
						 session.channel );
}