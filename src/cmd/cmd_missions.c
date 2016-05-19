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

#include <wb_cmd.h>
#include <wb_tools.h>
#include <wb_session.h>
#include <wb_list.h>
#include <wb_mission.h>
#include <wb_xmpp_wf.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

struct cb_args
{
	f_cmd_missions_cb cb;
	void *args;
};

static void cbm ( struct mission *m, void *args )
{
	if ( m->crown_time_gold == 0 )
		return;

	struct cb_args *a = ( struct cb_args * ) args;

	char *setting = strdup ( m->setting );
	char *p = strstr ( setting, "/" );

	if ( p != NULL )
		*p = 0;

	a->cb ( m->type, setting, m, a->args );

	free ( setting );
}

void cmd_missions ( f_cmd_missions_cb cb, void *args )
{
	if ( cb == NULL )
		return;

	struct cb_args a = { cb, args };

	list_foreach ( session.wf.missions, (f_list_callback) cbm, &a );
}

void cmd_missions_whisper_cb ( const char *type,
							   const char *setting,
struct mission *m,
	void *args )
{
	struct whisper_cb_args *a = ( struct whisper_cb_args * ) args;

	if ( a != NULL && a->nick_to != NULL && a->jid_to != NULL )
	{

		char *answer;
		FORMAT ( answer, "%s %s time %i:%02i ks %i",
				 type,
				 setting,
				 m->crown_time_gold / 60,
				 m->crown_time_gold % 60,
				 m->crown_perf_gold );

		xmpp_send_message ( a->nick_to, a->jid_to, answer );

		free ( answer );
	}
}

void cmd_missions_room_cb ( const char *type,
							const char *setting,
struct mission *m,
	void *args )
{
	char *reply;
	FORMAT ( reply, "%-16s %-10s\ttime: %i:%02i\tcrown: %i",
			 type,
			 setting,
			 m->crown_time_gold / 60,
			 m->crown_time_gold % 60,
			 m->crown_perf_gold );

	cmd_say ( reply );
	sleep ( 3 );
}

void cmd_missions_console_cb ( const char *type,
							   const char *setting,
struct mission *m,
	void *args )
{
	LOGPRINT ( "- %-16s %-10s\ttime: %i:%02i\tcrown: %i\n",
			   type,
			   setting,
			   m->crown_time_gold / 60,
			   m->crown_time_gold % 60,
			   m->crown_perf_gold );
}