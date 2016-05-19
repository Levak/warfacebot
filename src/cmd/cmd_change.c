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

#include <wb_pvp_maps.h>

void cmd_change ( const char *mission_name )
{
	struct mission *m = NULL;

	if ( strstr ( session.online.channel, "pvp" ) )
	{

		if ( mission_name == NULL )
			mission_name = "tdm_airbase";

		m = mission_list_get ( mission_name );

		if ( m != NULL )
		{
			xmpp_iq_gameroom_update_pvp ( m->mission_key,
										  PVP_AUTOBALANCE | PVP_DEADCHAT,
										  16, 0, NULL, NULL );
		}
		else
		{
			xmpp_iq_gameroom_update_pvp ( mission_name,
										  PVP_AUTOBALANCE | PVP_DEADCHAT,
										  16, 0, NULL, NULL );
		}
	}
	else
	{
		if ( mission_name == NULL )
			mission_name = "trainingmission";

		m = mission_list_get ( mission_name );

		if ( m != NULL )
		{
			xmpp_iq_gameroom_setinfo ( m->mission_key, NULL, NULL );
		}
		else
		{
			xmpp_iq_gameroom_setinfo ( mission_name, NULL, NULL );
		}
	}
}