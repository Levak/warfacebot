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
#include <wb_session.h>
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>

#include <stdio.h>
#include <stdlib.h>

typedef struct
{
	f_iq_cb fun;
	void *args;
}gameroom_open_cb_args_t;

static void xmpp_iq_gameroom_open_cb ( const char *msg, void *args )
{
	/* Answer :
	<iq to='masterserver@warface/pve_2' type='get'>
	<query xmlns='urn:cryonline:k01'>
	<data query_name='gameroom_open' compressedData='...'
	originalSize='42'/>
	</query>
	</iq>
	*/

	gameroom_open_cb_args_t *a = (gameroom_open_cb_args_t*) args;

	if ( xmpp_is_error ( msg ) )
		return;

	/* Leave previous room if any */
	if ( session.room_jid != NULL )
		xmpp_presence ( session.room_jid, 1 );

	xmpp_iq_player_status ( STATUS_ONLINE | STATUS_ROOM );

	char *data = wf_get_query_content ( msg );
	char *room = get_info ( data, "room_id='", "'", NULL );

	/* Join XMPP room */
	char *room_jid;

	FORMAT ( room_jid, "room.%s.%s@conference.warface", session.channel, room );
	xmpp_presence ( room_jid, 0 );

	if ( a && a->fun )
		a->fun ( a->args );

	free ( room_jid );

	free ( room );
	free ( data );
}

void xmpp_iq_gameroom_open ( f_iq_cb fun, void *args )
{
	gameroom_open_cb_args_t *a = calloc ( 1, sizeof *a );
	a->args = args;
	a->fun = fun;
	cb_args3_t *info = (cb_args3_t *) args;
	t_uid id;

	idh_generate_unique_id ( &id );
	idh_register ( &id, 0, xmpp_iq_gameroom_open_cb, a );

	LOGPRINT ( "%-16s "BOLD"%s\n", "CREATE GAME ROOM", (char*)info->first );

	/* Open the game room */
	send_stream_format ( session.wfs,
						 "<iq id='%s' to='masterserver@warface/%s' type='get'>"
						 " <query xmlns='urn:cryonline:k01'>"
						 "  <gameroom_open"
						 "      room_name='Room' team_id='1' status='1'"
						 "      class_id='1' room_type='1' private='1'"
						 "      mission='%s' inventory_slot='0'>"
						 "  </gameroom_open>"
						 " </query>"
						 "</iq>",
						 &id, session.channel, (char*)info->second );
}

