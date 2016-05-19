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
#include <wb_dbus.h>
#include <wb_friend.h>
#include <wb_clanmate.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

void profile_info_get_status_cb ( const char *info, void *friend )
{
	struct friend *f = ( struct friend* ) friend;
	if ( !info )
	{
		f->status = STATUS_OFFLINE;
		LOGPRINT ( "%-20s " KCYN BOLD "%s\n", "PLAYER OFFLINE", f->nickname );
		struct clanmate *c = ( struct clanmate* ) list_get ( session.profile.friends, f->nickname );
		if ( c )
			c->status = STATUS_OFFLINE;
	}
}

void friend_list_cb ( void *friend, void *args )
{
	struct friend *f = ( struct friend* ) friend;
	if ( f->status & STATUS_ONLINE )
		xmpp_iq_peer_status_update ( f );
		//xmpp_iq_profile_info_get_status ( f->nickname, profile_info_get_status_cb, f );
}

void clanmate_list_cb ( void *c, void *args )
{
	struct clanmate *f = ( struct clanmate* ) c;
	if ( list_get ( session.profile.friends, f->nickname ) )
		return;
	if ( f->status & STATUS_ONLINE )
		xmpp_iq_peer_clan_member_update ( f );
		//xmpp_iq_profile_info_get_status ( f->nickname, profile_info_get_status_cb, f );
}

void sigint_handler_ ( int signum )
{
	session.active = 0;

	pthread_exit ( NULL );
}

void *thread_refresh ( void *varg )
{
	signal ( SIGINT, sigint_handler_ );

	while ( 1 )
	{
		list_foreach ( session.profile.friends, friend_list_cb, NULL );
		list_foreach ( session.clan.clanmates, clanmate_list_cb, NULL );
		sleep ( 30 );
	}

	pthread_exit ( NULL );
}

static pthread_t th_refresh;

static void xmpp_iq_get_account_profiles_cb ( const char *msg,
enum xmpp_msg_type type,
	void *args )
{
	/* Answer :
	   <iq from="masterserver@warface/pve_12" type="result">
		 <query xmlns="urn:cryonline:k01">
		   <get_account_profiles>
			 <profile id="XXX" nickname="XXX"/>
		   </get_account_profiles>
		 </query>
	   </iq>
	*/

	if ( type & XMPP_TYPE_ERROR )
	{
		fprintf ( stderr, "Failed to get account profiles\nReason: " );

		int code = get_info_int ( msg, "code='", "'", NULL );
		int custom_code = get_info_int ( msg, "custom_code='", "'", NULL );

		switch ( code )
		{
			case 8:
				switch ( custom_code )
				{
					case 1:
						fprintf ( stderr, "Game version mismatch (%s)\n",
								  game_version_get ( ) );
						return;
					default:
						fprintf ( stderr, "Invalid user_id or active_token\n" );
						return;
				}
			default:
				fprintf ( stderr, "Unknown\n" );
				return;
		}
	}

	free ( session.profile.id );
	free ( session.profile.nickname );

	session.profile.id = get_info ( msg, "profile id='", "'", "PROFILE ID" );
	session.profile.nickname = get_info ( msg, "nickname='", "'", "NICKNAME" );

	if ( session.profile.id == NULL )
		xmpp_iq_create_profile ( );
	else
	{
		xmpp_iq_join_channel ( NULL, NULL, NULL );
#ifdef DBUS_API
		dbus_api_setup ( );
#endif
	}

	if ( pthread_create ( &th_refresh, NULL, &thread_refresh, NULL ) == -1 )
		perror ( "pthread_create" );
	else
		pthread_detach ( th_refresh );
}

void xmpp_iq_get_account_profiles ( void )
{
	t_uid id;

	idh_generate_unique_id ( &id );
	idh_register ( &id, 0, xmpp_iq_get_account_profiles_cb, NULL );

	/* Get CryOnline profile */

	send_stream_format ( session.wfs,
						 "<iq id='%s' to='ms.warface' type='get'>"
						 " <query xmlns='urn:cryonline:k01'>"
						 "  <get_account_profiles version='%s'"
						 "    user_id='%s' token='%s'/>"
						 " </query>"
						 "</iq>",
						 &id, game_version_get ( ),
						 session.online.id, session.online.active_token );
}