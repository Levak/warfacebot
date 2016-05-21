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

#include <string.h>

#include <wb_session.h>
#include <wb_mission.h>
#include <wb_xmpp_wf.h>
#include <wb_tools.h>

void cmd_open ( const char *mission_name )
{
	if ( mission_name == NULL )
		mission_name = "trainingmission";

	if ( strlen ( mission_name ) != 36 ) /* not an uuid */
	{
		struct mission *m = mission_list_get ( mission_name );

		if ( m != NULL )
		{
			int is_pvp = strstr ( m->mode, "pvp" ) != NULL;
			int were_in_pvp = strstr ( session.online.channel, "pvp" ) != NULL;

			if ( is_pvp && !were_in_pvp )
				xmpp_iq_join_channel ( "pvp_pro_5", NULL, NULL );
			else if ( !is_pvp && were_in_pvp )
				xmpp_iq_join_channel ( "pve_2", NULL, NULL );

			if ( is_pvp )
				xmpp_iq_gameroom_open ( m->mission_key, ROOM_PVP_PUBLIC, NULL, NULL );
			else
				xmpp_iq_gameroom_open ( m->mission_key, ROOM_PVE_PRIVATE, NULL, NULL );
		}
		else
		{
			LOGPRINT ( KRED "NO SUCH MAP OR MISSION\n" );
		}
	}
	else
	{
		xmpp_iq_gameroom_open ( mission_name, ROOM_PVE_PRIVATE, NULL, NULL );
	}
}