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
	session.gameroom.leaving = 0;
	session.gameroom.joined = 0;
	session.commands.farming = 0;
	session.commands.silent = 0;
	session.commands.notify = 0;
	session.xmpp.last_query = time ( NULL );
	session.commands.whitelist = NULL;

	session.clan.own_position = 1;			/* Needs to be removed later*/
	session.clan.leaderboard_position = 999999;

	session.commands.cmd_list = malloc ( sizeof ( char * ) );
	session.commands.cmd_list_size = 0;

	session.log.output = fLog;
	session.log.debug = fDebug;

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

	free ( session.xmpp.jid );
	free ( session.profile.nickname );
	free ( session.online.active_token );
	free ( session.profile.id );
	free ( session.online.id );
	free ( session.online.channel );
	free ( session.clan.name );
	free ( session.commands.whitelist );

	if ( session.log.output )
	{
		fclose ( session.log.output );
		fclose ( session.log.debug );
	}
}