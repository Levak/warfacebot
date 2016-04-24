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

#include <wb_session.h>
#include <wb_list.h>
#include <wb_friend.h>
#include <wb_clanmate.h>
#include <wb_mission.h>
#include <wb_room.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct session session = { 0 };

void session_init ( int fd, FILE *fLog, FILE *fDebug )
{
	session.wfs = fd;
	session.active = 1;
	session.leaving = 0;
	session.ingameroom = 0;
	session.farming = 0;
	session.silent = 0;
	session.notify = 0;
	session.last_query = time ( NULL );
	session.whitelist = NULL;

	session.clan_own_position = 1;			/* Needs to be removed later*/
	session.clan_leaderboard_position = 999999;

	session.cmd_list = malloc ( sizeof ( char * ) );
	session.cmd_list_size = 0;

	session.fLog = fLog;
	session.fDebug = fDebug;

	friend_list_init ( );
	clanmate_list_init ( );
	mission_list_init ( );
	room_list_init ( );
}

void session_free ( void )
{
	friend_list_free ( );
	clanmate_list_free ( );
	mission_list_free ( );

	free ( session.jid );
	free ( session.nickname );
	free ( session.active_token );
	free ( session.profile_id );
	free ( session.online_id );
	free ( session.channel );
	free ( session.clan_name );
	free ( session.whitelist );

	if ( session.fLog )
	{
		fclose ( session.fLog );
		fclose ( session.fDebug );
	}
}