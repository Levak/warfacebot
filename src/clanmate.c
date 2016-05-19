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

#include <wb_clanmate.h>
#include <wb_session.h>
#include <wb_list.h>
#include <wb_dbus.h>
#include <wb_tools.h>
#include <wb_cmd.h>

#include <stdlib.h>
#include <string.h>

static int clanmate_cmp ( const struct clanmate *f, const char *nickname )
{
	/* Compare nicknames, because jid is not always available */
	return strcmp ( f->nickname, nickname );
}

static inline void clanmate_free_fields_ ( struct clanmate *f )
{
	free ( f->jid );
	free ( f->nickname );
	free ( f->profile_id );
}

static inline void clanmate_set_fields_ ( struct clanmate *f,
										  const char *jid,
										  const char *nickname,
										  const char *profile_id,
										  int status,
										  int experience,
										  int clan_points,
										  int clan_role,
										  unsigned int invite_date,
										  char *place_token,
										  char *place_info_token,
										  char *mode_info_token,
										  char *mission_info_token )
{
	f->jid = jid && *jid ? strdup ( jid ) : NULL;
	f->nickname = strdup ( nickname );
	f->profile_id = strdup ( profile_id );
	f->status = status;
	f->experience = experience;
	f->clan_points = clan_points;
	f->clan_role = clan_role;
	f->invite_date = invite_date;
	f->place_token = strdup ( place_token ? place_token : "" );
	f->place_info_token = strdup ( place_info_token ? place_info_token : "" );
	f->mode_info_token = strdup ( mode_info_token ? mode_info_token : "" );
	f->mission_info_token = strdup ( mission_info_token ? mission_info_token : "" );
}

static void clanmate_free ( struct clanmate *f )
{
	clanmate_free_fields_ ( f );
	free ( f );
}

struct clanmate *clanmate_list_add ( const char *jid,
									 const char *nickname,
									 const char *profile_id,
									 int status,
									 int experience,
									 int clan_points,
									 int clan_role,
									 unsigned int invite_date )
{
	struct clanmate *f = calloc ( 1, sizeof ( struct clanmate ) );

	clanmate_set_fields_ ( f, jid, nickname, profile_id, status, experience,
						   clan_points, clan_role, invite_date, "", "", "", "" );

	list_add ( session.clan.clanmates, f );

	cmd_list_add ( "whisper %s ", nickname );
	cmd_list_add ( "invite %s", nickname );
	cmd_list_add ( "follow %s", nickname );
	cmd_list_add ( "master %s", nickname );
	cmd_list_add ( "whois %s", nickname );
	cmd_list_add ( "whitelist %s", nickname );

	return f;
}

void clanmate_list_update ( const char *jid,
							const char *nickname,
							const char *profile_id,
							int status,
							int experience,
							int clan_points,
							int clan_role,
							unsigned int invite_date,
							char *place_token,
							char *place_info_token,
							char *mode_info_token,
							char *mission_info_token )
{
	/* Handled by friend_list_update() */
	if ( list_get ( session.profile.friends, nickname ) )
		return;

	struct clanmate *f = list_get ( session.clan.clanmates, nickname );

	if ( !f )
		return;

	if ( status & STATUS_ONLINE )
	{
		char *map;
		if ( place_token && place_info_token && strstr ( place_token, "pve" ) )
			FORMAT ( map, "%s", place_info_token );
		else if ( place_token && mission_info_token && strstr ( place_token, "pvp" ) )
			FORMAT ( map, "@%s", mission_info_token + sizeof ( "@pvp_mission_display_name" ) );
		else
			map = strdup ( KRED "@unknown" );

		int NOTONLINE = STATUS_PLAYING | STATUS_AFK | STATUS_ROOM;

		if ( f->status == STATUS_OFFLINE )
			LOGPRINT ( "%-20s " KGRN BOLD "%s\n", "PLAYER ONLINE", nickname );
		else if ( ( f->status & NOTONLINE ) && !( status & NOTONLINE ) )
			LOGPRINT ( "%-20s " KGRN BOLD "%s\n", "PLAYER ONLINE", nickname );
		else if ( ( !( f->status & STATUS_AFK ) || ( f->status & ( STATUS_ROOM | STATUS_PLAYING ) ) ) &&
				  ( ( status & STATUS_AFK ) && !( status & ( STATUS_ROOM | STATUS_PLAYING ) ) ) )
			LOGPRINT ( "%-20s " KYEL BOLD "%s\n", "PLAYER AFK", nickname );
		else if ( !( f->status & STATUS_PLAYING ) && ( status & STATUS_PLAYING ) )
			LOGPRINT ( "%-20s " KMAG BOLD "%-20s " KRST BOLD "%s\n", "PLAYER INGAME", nickname, map );
		else if ( !( f->status & STATUS_ROOM ) && ( status & STATUS_ROOM ) )
			LOGPRINT ( "%-20s " KGRN BOLD "%-20s " KRST BOLD "%s\n", "PLAYER INROOM", nickname, map );

		free ( map );
	}
	else if ( f->status & STATUS_ONLINE )
		LOGPRINT ( "%-20s " KCYN BOLD "%s\n", "PLAYER OFFLINE", nickname );

	clanmate_free_fields_ ( f );

	clanmate_set_fields_ ( f, jid, nickname, profile_id, status, experience,
						   clan_points, clan_role, invite_date,
						   place_token, place_info_token, mode_info_token, mission_info_token );

#ifdef DBUS_API
	dbus_api_update_buddy_list ( );
#endif
}

void clanmate_list_remove ( const char *nickname )
{
	list_remove ( session.clan.clanmates, nickname );
}

void clanmate_list_empty ( void )
{
	list_empty ( session.clan.clanmates );
}

void clanmate_list_init ( void )
{
	session.clan.clanmates = list_new ( (f_list_cmp) clanmate_cmp,
								   (f_list_free) clanmate_free );
}

void clanmate_list_free ( void )
{
	list_free ( session.clan.clanmates );
	session.clan.clanmates = NULL;
}