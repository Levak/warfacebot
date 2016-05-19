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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#include <readline/readline.h>
#include <readline/history.h>

#include <wb_game.h>
#include <wb_stream.h>
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>
#include <wb_tools.h>
#include <wb_session.h>
#include <wb_cmd.h>
#include <wb_dbus.h>

 /** THREADS **/

#ifdef __MINGW32__
#include <windows.h>
#define sleep(x) Sleep(x)
#endif

void idle_close ( const char *name );

void sigint_handler ( int signum )
{
	session.active = 0;

#ifdef DBUS_API
	dbus_api_quit ( 0 );
#endif

	pthread_exit ( NULL );
}

void register_sigint_handler ( void )
{
	signal ( SIGINT, sigint_handler );
}

static int cmd_1arg ( char *cmdline, char **arg1 )
{
	char *saveptr = NULL;

	if ( cmdline == NULL )
		return 0;

	*arg1 = strtok_r ( cmdline, "", &saveptr );

	return *arg1 != NULL;
}

static int cmd_2args ( char *cmdline, char **arg1, char **arg2 )
{
	char *saveptr = NULL;

	if ( cmdline == NULL )
		return 0;

	*arg1 = strtok_r ( cmdline, " ", &saveptr );
	*arg2 = strtok_r ( NULL, "", &saveptr );

	return *arg2 != NULL;
}

#if 0
static int cmd_3args ( char *cmdline, char **arg1, char **arg2, char **arg3 )
{
	char *saveptr = NULL;

	if ( cmdline == NULL )
		return 0;

	*arg1 = strtok_r ( cmdline, " ", &saveptr );
	*arg2 = strtok_r ( NULL, " ", &saveptr );
	*arg3 = strtok_r ( NULL, "", &saveptr );

	return *arg3 != NULL;
}
#endif

struct farm_args
{
	int n;
	char *master;
	char *players[ 3 ];
	char *mission_name;
};

void *thread_farm ( void *varg )
{
	struct farm_args *farm_args = ( struct farm_args* )varg;
	int played = 0;
	while ( session.commands.farming )
	{
		cmd_open ( farm_args->mission_name );
		sleep ( 3 );

		cmd_invite ( farm_args->master, 0 );
		sleep ( 1 );
		cmd_master ( farm_args->master );

		for ( int i = 0; i != farm_args->n; ++i )
			cmd_invite ( farm_args->players[ i ], 0 );
		sleep ( 4 );

		cmd_ready ( NULL );
		cmd_say ( "go" );

		while ( session.gameroom.joined )
			sleep ( 1 );

		xmpp_iq_join_channel ( NULL, NULL, NULL );

		LOGPRINT ( BOLD "%d " KRST "games done.\n", ++played );
	}

	pthread_exit ( NULL );
}

void invite_master_cb ( const char *msg_id,
						const char *msg,
						void *args )
{
	struct farm_args *farm_args = ( struct farm_args* )args;

	cmd_master ( farm_args->master );

	for ( int i = 0; i != farm_args->n; ++i )
		cmd_invite ( farm_args->players[ i ], 0 );
}

void gameroom_open_farm_cb ( const char *room_id, void *args )
{
	struct farm_args *farm_args = ( struct farm_args* )args;

	xmpp_iq_invitation_send ( farm_args->master, 0, invite_master_cb, args );
}

void join_channel_farm_cb ( void *args )
{
	struct farm_args *farm_args = ( struct farm_args* )args;

	struct mission *m = ( struct mission* )mission_list_get ( farm_args->mission_name );

	xmpp_iq_gameroom_open ( m->mission_key, ROOM_PVE_PRIVATE,
							gameroom_open_farm_cb, args );
}

void *thread_farm_fast ( void *varg )
{
	register_sigint_handler ( );

	struct farm_args *farm_args = ( struct farm_args* )varg;
	int played = 0;
	struct mission *m = mission_list_get ( farm_args->mission_name );
	char *mission_key = strdup ( m->mission_key );

	while ( session.commands.farming )
	{
		int were_in_pvp = strstr ( session.online.channel, "pvp" ) != NULL;
		if ( were_in_pvp )
			xmpp_iq_join_channel ( "pve_2", join_channel_farm_cb, varg );
		else
			xmpp_iq_gameroom_open ( mission_key, ROOM_PVE_PRIVATE,
									gameroom_open_farm_cb, varg );

		sleep ( 6 );

		while ( session.gameroom.joined )
			sleep ( 1 );

		LOGPRINT ( BOLD "%d " KRST "games done.\n", ++played );
	}

	free ( mission_key );
	pthread_exit ( NULL );
}

const char *cmd_list_[ ] = { "add ", "channel", "whois ", "missions", "say", "open ", "name ", "change", "ready", "invite ", "friends", "follow ", "master ", "start", "switch", "farm ", "leave", "silent", "whitelist none", "notify", "randombox " };

// Generator function for word completion.
char *my_generator ( const char *text, int state )
{
	static unsigned list_index, len;
	const char *name;

	if ( !state )
	{
		list_index = 0;
		len = strlen ( text );
	}
	
	while ( ( name = session.commands.cmd_list[ list_index ] ) &&
			list_index < session.commands.cmd_list_size )
	{
		list_index++;
		if ( !strncmp ( name, text, len ) )
			return strdup ( name );
	}

	// If no names matched, then return NULL.
	return NULL;
}

// Custom completion function
static char **my_completion ( const char *text, int start, int end )
{
	// This prevents appending space to the end of the matching word
	rl_completion_append_character = '\0';

	char **matches = NULL;
	if ( start == 0 )
		matches = rl_completion_matches ( text, &my_generator );
	rl_attempted_completion_over = 1;

	return matches;
}

void *thread_readline ( void *varg )
{
	int wfs = session.wfs;

	register_sigint_handler ( );
	using_history ( );
	rl_attempted_completion_function = my_completion;
	rl_completer_word_break_characters = "\t\n\"\\'`@$><=;|&{(";

	for ( int i = 0; i != sizeof ( cmd_list_ ) / sizeof ( *cmd_list_ ); ++i )
		cmd_list_add ( cmd_list_[ i ] );

	do
	{
		char *buff_readline = readline ( "CMD>" );

		if ( buff_readline == NULL )
		{
			if ( session.active )
			{
				xmpp_iq_player_status ( STATUS_OFFLINE );
			}

			free ( buff_readline );
			break;
		}

		int buff_size = strlen ( buff_readline );

		if ( buff_size <= 1 )
			flush_stream ( wfs );
		else
		{
			add_history ( buff_readline );

			if ( buff_readline[ 0 ] != '<' )
			{
				char *cmd;
				char *args;

				cmd_2args ( buff_readline, &cmd, &args );

				if ( strstr ( cmd, "remove" ) )
				{
					char *nickname;

					if ( cmd_1arg ( args, &nickname ) )
						cmd_remove_friend ( nickname );
				}

				else if ( strstr ( cmd, "add" ) )
				{
					char *nickname;

					if ( cmd_1arg ( args, &nickname ) )
						cmd_add_friend ( nickname );
				}

				else if ( strstr ( cmd, "channel" ) )
				{
					char *channel;

					if ( cmd_1arg ( args, &channel ) )
						cmd_channel ( channel );
				}

				else if ( strstr ( cmd, "whisper" ) )
				{
					char *nickname;
					char *message;

					if ( cmd_2args ( args, &nickname, &message ) )
						cmd_whisper ( nickname, message );
				}

				else if ( strstr ( cmd, "whois" ) )
				{
					char *nickname;

					if ( cmd_1arg ( args, &nickname ) )
						cmd_whois ( nickname, cmd_whois_console_cb, NULL );
				}

				else if ( strstr ( cmd, "missions" ) )
				{
					cmd_missions ( cmd_missions_console_cb, NULL );
				}

				else if ( strstr ( cmd, "say" ) )
				{
					char *message;

					if ( cmd_1arg ( args, &message ) )
						cmd_say ( message );
				}

				else if ( strstr ( cmd, "open" ) )
				{
					char *mission;

					if ( cmd_1arg ( args, &mission ) )
						cmd_open ( mission );
					else
						cmd_open ( NULL );
				}

				else if ( strstr ( cmd, "name" ) )
				{
					char *name;

					if ( cmd_1arg ( args, &name ) )
						cmd_name ( name );
				}

				else if ( strstr ( cmd, "change" ) )
				{
					char *mission;

					if ( cmd_1arg ( args, &mission ) )
						cmd_change ( mission );
					else
						cmd_change ( NULL );
				}

				else if ( strstr ( cmd, "ready" ) )
				{
					char *class;

					if ( cmd_1arg ( args, &class ) )
						cmd_ready ( class );
					else
						cmd_ready ( NULL );
				}

				else if ( strstr ( cmd, "invite" ) )
				{
					char *nickname;

					if ( cmd_1arg ( args, &nickname ) )
						cmd_invite ( nickname, 0 );
				}

				else if ( strstr ( cmd, "friends" ) )
				{
					cmd_friends ( );
				}

				else if ( strstr ( cmd, "follow" ) )
				{
					char *nickname;

					if ( cmd_1arg ( args, &nickname ) )
						cmd_follow ( nickname );
				}

				else if ( strstr ( cmd, "master" ) )
				{
					char *nickname;

					if ( cmd_1arg ( args, &nickname ) )
						cmd_master ( nickname );
				}

				else if ( strstr ( cmd, "start" ) )
				{
					cmd_start ( );
				}

				else if ( strstr ( cmd, "stats" ) )
				{
					cmd_stats ( cmd_stats_console_cb, NULL );
				}

				else if ( strstr ( cmd, "switch" ) )
				{
					cmd_switch ( );
				}

				else if ( strstr ( cmd, "leave" ) )
				{
					cmd_leave ( );
				}

				else if ( strstr ( cmd, "safe" ) )
				{
					char *mission_name;

					if ( cmd_1arg ( args, &mission_name ) )
						cmd_safe ( mission_name );
					else
						cmd_safe ( "tdm_airbase" );
				}

				else if ( strstr ( cmd, "farm" ) )
				{
					session.commands.farming = !session.commands.farming;
					static pthread_t th_farm;
					if ( session.commands.farming )
					{
						char master[ 20 ] = { 0 };
						char players[ 3 ][ 20 ];
						char mission_name[ 33 ] = { 0 };
						int n = sscanf ( args, "%s %s %s %s %s", mission_name,
										 master, players[ 0 ], players[ 1 ], players[ 2 ] ) - 2;
						LOGPRINT ( "%-20s " KGRN BOLD "%-16s " KRST KGRN " %-16s %-16s %-16s\n",
								   "FARMING WITH", master, players[ 0 ], players[ 1 ], players[ 2 ] );
						FORMAT ( session.commands.whitelist, "%s %s %s %s", master,
								 players[ 0 ], players[ 1 ], players[ 2 ] );

						struct farm_args *farm_args = malloc ( sizeof ( struct farm_args ) );

						farm_args->n = n;
						farm_args->master = strdup ( master );
						for ( int i = 0; i != n; ++i )
							farm_args->players[ i ] = strdup ( players[ i ] );
						farm_args->mission_name = strdup ( mission_name );

						if ( pthread_create ( &th_farm, NULL, &thread_farm_fast, farm_args ) == -1 )
							perror ( "pthread_create" );
						else
							pthread_detach ( th_farm );
					}
					else
					{
						pthread_kill ( th_farm, 0 );
						free ( session.commands.whitelist );
						session.commands.whitelist = NULL;
						LOGPRINT ( KYEL "%s\n", "STOPPED FARMING" );
					}
				}

				else if ( strstr ( cmd, "silent" ) )
				{
					session.commands.silent = !session.commands.silent;
					if ( session.commands.silent )
						LOGPRINT ( KYEL BOLD "%s\n", "SILENT" );
					else
						LOGPRINT ( KGRN BOLD "%s\n", "LOUD" );
				}

				else if ( strstr ( cmd, "notify" ) )
				{
					session.commands.notify = !session.commands.notify;
					if ( !session.commands.notify )
						LOGPRINT ( KYEL BOLD "%s\n", "NOTIFICATIONS OFF" );
					else
						LOGPRINT ( KGRN BOLD "%s\n", "NOTIFICATIONS ON" );
				}

				else if ( strstr ( cmd, "whitelist" ) )
				{
					if ( args && *args )
						session.commands.whitelist = strdup ( args );
					else
						session.commands.whitelist = NULL;
				}

				else if ( strstr ( cmd, "randombox" ) )
				{
					char item[32], needed[32];
					int money;

					sscanf ( args, "%s %d %s", item, &money, needed );
					int moneyLeft = session.profile.money - money;

					cmd_randombox ( item, needed, moneyLeft );
				}

				else
					LOGPRINT ( KRED "Command not found: %s\n", cmd );
			}
			else
				send_stream ( wfs, buff_readline, buff_size );
			sleep ( 1 );
		}

		free ( buff_readline );
	} while ( session.active );

	idle_close ( "readline" );
	pthread_exit ( NULL );
}

#ifdef STAT_BOT

static void print_number_of_players_cb ( const char *msg,
enum xmpp_msg_type type,
	void *args )
{
	FILE *sfile = (FILE *) args;

	unsigned int count_all = 0;
	unsigned int count_pvp = 0;
	unsigned int count_pve = 0;

	const char *m = msg;
	while ( ( m = strstr ( m, "<server " ) ) )
	{
		/* Extract room jid */
		char *rjid = get_info ( m, "resource='", "'", NULL );

		if ( rjid != NULL )
		{
			unsigned int count = get_info_int ( m, "online='", "'", NULL );

			if ( strstr ( rjid, "pve" ) )
				count_pve += count;
			else if ( strstr ( rjid, "pvp" ) )
				count_pvp += count;
			count_all += count;
		}

		free ( rjid );
		++m;
	}

	fprintf ( sfile, "%u,%u,%u,%u\n",
			  (unsigned) time ( NULL ), count_all, count_pve, count_pvp );

	fflush ( sfile );
}

void *thread_stats ( void *varg )
{
	int wfs = session.wfs;
	FILE *sfile = stdout;

	/* Dirty hack to wait for session initialisation */
	sleep ( 3 );

	{
		char *s;

		FORMAT ( s, "stats-%s-%ld.csv",
				 game_server_get_str ( ),
				 time ( NULL ) );

		sfile = fopen ( s, "w" );

		if ( sfile == NULL )
		{
			fprintf ( stderr, "Unable to open %s for writting\n", s );
			sfile = stdout;
		}

		free ( s );
	}

	fprintf ( sfile, "Time,All players,PvE players,PvP players\n" );

	register_sigint_handler ( );

	idh_register ( ( t_uid * ) "stats_num", 1, &print_number_of_players_cb, sfile );

	do
	{
		send_stream_ascii ( wfs,
							"<iq to='k01.warface' type='get' id='stats_num'>"
							"<query xmlns='urn:cryonline:k01'>"
							"<get_master_servers/>"
							"</query>"
							"</iq>" );
		flush_stream ( wfs );
		sleep ( 5 );
	} while ( session.active );


	if ( sfile != stdout )
	{
		fclose ( sfile );
	}

	printf ( "Closed stats\n" );
	pthread_exit ( NULL );
}
#endif

void *thread_dispatch ( void *vargs )
{
	register_sigint_handler ( );

	XMPP_REGISTER_QUERY_HDLR ( );
	XMPP_WF_REGISTER_QUERY_HDLR ( );

	do
	{
		char *msg = read_stream ( session.wfs );

		if ( msg == NULL || strlen ( msg ) <= 0 )
		{
			if ( session.active )
			{
				xmpp_iq_player_status ( STATUS_OFFLINE );
			}

			break;
		}
		{ /* Replace any " with ' */
			for ( char *s = msg; *s; ++s )
				if ( *s == '"' )
					*s = '\'';
		}
		char *msg_id = get_msg_id ( msg );
		enum xmpp_msg_type type = get_msg_type ( msg );

		/* If we expect an answer from that ID */
		if ( msg_id != NULL && idh_handle ( msg_id, msg, type ) )
		{
			/* Good, we handled it */
		}
		/* If someone thinks we expected an answer */
		else if ( type & ( XMPP_TYPE_ERROR | XMPP_TYPE_RESULT ) )
		{
#ifdef DEBUG
			if ( msg_id != NULL )
			{
				/* Unhandled stanza */
				fprintf ( stderr, "FIXME - Unhandled id: %s\n%s\n", msg_id, msg );

				if ( session.log.debug )
				{
					fflush ( session.log.debug );
					fprintf ( session.log.debug, KWHT BOLD "[%s]  " KRST, get_timestamp ( ) );
					fprintf ( session.log.debug, "FIXME - Unhandled id: %s\n%s\n", msg_id, msg );
					fflush ( session.log.debug );
				}
			}
#endif
		}
		/* If it wasn't handled and it's not a result */
		else
		{
			char *stanza = get_query_tag_name ( msg );

			if ( stanza == NULL )
			{
#ifdef DEBUG
				fprintf ( stderr, "FIXME - Unhandled msg:\n%s\n", msg );

				if ( session.log.debug )
				{
					fflush ( session.log.debug );
					fprintf ( session.log.debug, KWHT BOLD "[%s]  " KRST, get_timestamp ( ) );
					fprintf ( session.log.debug, "FIXME - Unhandled msg:\n%s\n", msg );
					fflush ( session.log.debug );
				}
#endif
			}
			/* Look if tagname is registered */
			else if ( qh_handle ( stanza, msg_id, msg ) )
			{
				/* Good, we handled it */
			}
			else
			{
#ifdef DEBUG
				/* Unhandled stanza */
				fprintf ( stderr, "FIXME - Unhandled query: %s\n%s\n", stanza, msg );

				if ( session.log.debug )
				{
					fflush ( session.log.debug );
					fprintf ( session.log.debug, KWHT BOLD "[%s]  " KRST, get_timestamp ( ) );
					fprintf ( session.log.debug, "FIXME - Unhandled query: %s\n%s\n", stanza, msg );
					fflush ( session.log.debug );
				}
#endif
			}

			free ( stanza );
		}

		free ( msg );
		free ( msg_id );

		session.xmpp.last_query = time ( NULL );

	} while ( session.active );

	idle_close ( "dispatch" );
	pthread_exit ( NULL );
}

void *thread_ping ( void *vargs )
{
	int previous_ping = 0;
	const int ping_delay = 1 * 60;

	register_sigint_handler ( );

	do
	{

		if ( session.xmpp.last_query + 4 * ping_delay < time ( NULL ) )
		{
			LOGPRINT ( "%s", KRED "It's over.\n\n" );
			break;
		}
		else if ( session.xmpp.last_query + 3 * ping_delay < time ( NULL ) )
		{
			LOGPRINT ( "%s", KYEL "Stalling life... " );
			xmpp_iq_ping ( );
			previous_ping = 1;
		}
		else if ( previous_ping )
		{
			LOGPRINT ( "%s", KYEL "Still there!\n" );
			previous_ping = 0;
		}

		sleep ( ping_delay );

	} while ( session.active );

	idle_close ( "ping" );
	pthread_exit ( NULL );
}

static pthread_t th_dispatch;
static pthread_t th_readline;
static pthread_t th_ping;

void idle_init ( void )
{
	if ( pthread_create ( &th_dispatch, NULL, &thread_dispatch, NULL ) == -1 )
		perror ( "pthread_create" );
	else
		pthread_detach ( th_dispatch );

#if ! defined DBUS_API || defined DEBUG
	if ( pthread_create ( &th_readline, NULL, &thread_readline, NULL ) == -1 )
		perror ( "pthread_create" );
	else
		pthread_detach ( th_readline );
#endif

	if ( pthread_create ( &th_ping, NULL, &thread_ping, NULL ) == -1 )
		perror ( "pthread_create" );
	else
		pthread_detach ( th_ping );

#ifdef STAT_BOT
	{
		pthread_t thread_dl;

		if ( pthread_create ( &thread_dl, NULL, &thread_stats, NULL ) == -1 )
			perror ( "pthread_create" );

		pthread_detach ( thread_dl );
	}

#endif
}

void idle_run ( void )
{
#ifdef DBUS_API
	dbus_api_enter ( );
#else
	while ( session.active )
		sleep ( 1 );
#endif

	LOGPRINT ( "%s", BOLD "Closed idle\n" );
}

void idle_close ( const char *name )
{
	LOGPRINT ( "Closed %s\n", name );

#ifdef DBUS_API
	dbus_api_quit ( 0 );
#endif

	session.active = 0;

	pthread_kill ( th_ping, SIGINT );
	pthread_kill ( th_dispatch, SIGINT );
	pthread_kill ( th_readline, SIGINT );

	exit ( 1 );
}

int main ( int argc, char *argv[ ] )
{
	if ( argc <= 2 )
	{
		LOGPRINT ( "%s", KRED BOLD "USAGE: ./wb token online_id [eu|na|tr|vn|ru [version [server]]]\n" );

		return 2;
	}

	char *token = argv[ 1 ];
	char *online_id = argv[ 2 ];
	enum e_server server = SERVER_EU;

	if ( argc > 3 )
	{
		if ( strcmp ( argv[ 3 ], "eu" ) == 0 )
			server = SERVER_EU;
		else if ( strcmp ( argv[ 3 ], "na" ) == 0 )
			server = SERVER_NA;
		else if ( strcmp ( argv[ 3 ], "tr" ) == 0 )
			server = SERVER_TR;
		else if ( strcmp ( argv[ 3 ], "ru" ) == 0 )
			server = SERVER_RU;
		else if ( strcmp ( argv[ 3 ], "br" ) == 0 )
			server = SERVER_BR;
		/*        else if (strcmp(argv[3], "cn") == 0)
				  server = SERVER_CN;*/
		else if ( strcmp ( argv[ 3 ], "vn" ) == 0 )
			server = SERVER_VN;
		else
		{
			fprintf ( stderr,
					  "Unknown server '%s', falling back on EU.\n",
					  argv[ 3 ] );

			server = SERVER_EU;
		}
	}

	game_set ( server );

	FILE *fLog = NULL;
	FILE *fDebug = NULL;
	if ( argc > 4 && strlen ( argv[ 4 ] ) > 4 )
	{
		char *szLogFile, *szDeugFile;
		FORMAT ( szLogFile, "%s.log", argv[ 4 ] );
		FORMAT ( szDeugFile, "%s_debug.log", argv[ 4 ] );
		fLog = fopen ( szLogFile, "a" );
		fDebug = fopen ( szDeugFile, "a" );
		free ( szLogFile );
		free ( szDeugFile );
	}

	if ( argc > 5 )
	{
		game_version_set ( argv[ 5 ] );
	}

	if ( argc > 6 )
	{
		game_xmpp_server_set ( argv[ 6 ] );
	}

	/* Start of -- Legal Notices */

	printf ( BOLD "Warfacebot Copyright (C) 2015, 2016 Levak Borok\n"
			 "This program comes with ABSOLUTELY NO WARRANTY.\n"
			 "This is free software, and you are welcome to redistribute it\n"
			 "under certain conditions; see AGPLv3 Terms for details.\n\n" );

	/* End of -- Legal Notices */

	int wfs = connect_wf ( game_xmpp_server_get ( ), 5222 );

	if ( wfs > 0 )
	{
		session_init ( wfs, fLog, fDebug );

		idle_init ( );

		xmpp_connect ( token, online_id );

		idle_run ( );

		xmpp_close ( );

		session_free ( );
	}

	game_free ( );

	LOGPRINT ( "%s", BOLD "Warface Bot closed!\n" );

	return 0;
}

/* Unresolved Queries:
-----------------------

aas_notify_playtime
autorotate
confirm_notification
expire_profile_items profile_idle class_id time_played
external_shop_confirm_query supplierId orderId
external_shop_refund_query supplierId orderId
invitation_accept
invitation_send
lobbychat_getchannelid channel
map_voting_finished
map_voting_started
map_voting_state
map_voting_vote mission_uid
mission_load
mission_load_failed
mission_unload
mission_update
notification_broadcast
on_voting_finished
on_voting_started
on_voting_vote
persistent_settings_get
persistent_settings_set
preinvte_response
quickplay_maplist received size
remove_friend target=nickname
resync_profile
send_invitation target type
set_banner
shop_get_offers
shop_buy_offer supplier_id offer_id
shop_buy_multiple_offer supplier_id/offer id
shop_buy_external_offer supplier_id offer_id
shop_buy_multiple_external_offer supplier_id/offer id
sponsor_info_updated
srv_player_kicked
tutorial_status

validate_payer_info
voting_start target
voting_vote answer
set_reward_info session_id difficulty isPvE isClanWar mission_id incomplete_session session_time session_kill_counter winning_team_id passed_sublevels_counter passed_checkpoints_counter secondary_objectives_completed max_session_score/players_performance/stat id value
customhead_reset default_head
========= */

/* Results :
------------

update_cry_money cry_money

========= */

/* Queries for ms.warface :
---------------------------

channel_logout X
items
gameroom_setgameprogress room_id game_progress
gameroom_askserver server
gameroom_join room_id team_id group_id status class_id
gameroom_get room_type='14' size='108' received='0' cancelled='0' token='0'
gameroom_kick target_id
gameroom_leave
gameroom_loosemaster
gameroom_offer_response
gameroom_on_expired
gameroom_on_kicked
gameroom_open [mission_data] room_name team_id status class_id room_type private mission friendly_fire enemy_outlines auto_team_balance dead_can_chat join_in_the_process max_player inventory_slot class_riffleman(enabled) [...] / session?
gameroom_promote_to_host new_host_profile_id
gameroom_quickplay room_type room_name mission_id mission_type status team_id class_id missions_hash content_hash channel_switches? timestamp? uid? group?// session
gameroom_quickplay_cancel
gameroom_quickplay_canceled
gameroom_quickplay_failed
gameroom_quickplay_started
gameroom_quickplay_succeeded
gameroom_setinfo by_mission_key mission_key data
gameroom_setname room_name
gameroom_setplayer team_id status class_id
gameroom_setprivatestatus private
gameroom_setteamcolor
gameroom_switchteams
gameroom_sync
gameroom_update_pvp by_mission_key mission_key [data] private
generic_telemetry X
telemetry_stream session_id packet_id finalize
get_account_profiles version='1.1.1.3522' user_id='xxxxxxxx' token='$account_token'
get_achievements/achievement profile_id='xxxx'
get_configs
get_last_seen_date profile_id='xxxxxx'
get_player_stats
ingame_event profile_id event_name [data?key/value]// error
missions_get_list
validate_player_info online_id nickname banner_badge banner_mark banner_stripe experience clan_name clan_role clan_position clan_points clan_member_since
class_presence

======== */


/* Queries for masterserver@warface/pve_12 :
--------------------------------------------

telemetry_getleaderboard limit='10' // Removed ?
admin_cmd command='' args='' // kick ban unban mute unmute
get_contracts profile_id // doesn't work with other profile_id
get_cry_money
get_profile_performance
get_storage_items
session_join
session_join_dediclient
unbind_server_info server
extend_item item_id supplier_id offer_id
get_expired_items
update_achievements[cheat_mode=1]/achievement profile_id='xxxx'/chunk achievement_id='53' progress='15813349' completion_time='0'
update_contracts
abuse_report target type comment
peer_player_info to online_id@warface/GameClient
clan_info_sync
clan_list
getprofile session_id id
setcharacter gender(female/male) height fatness current_class
setcurrentclass current_class
ui_user_choice/choice choice_id choice_result

======== */


/* Queries for k01.warface :
----------------------------

account login=userid [password=token]
create_profile version='1.1.1.3522' token='$account_token' nickname='xxxxx' user_id='xxxxxxxx' resource='pve_12'
get_master_server rank channel resource used_resources is_quickplay
get_master_servers
join_channel version='1.1.1.3522' token='$account_token' profile_id='xxxx' user_id='xxxxxxxx' resource='pve_12' build_type='--release'
switch_channel version='1.1.1.3522' token='$account_token' profile_id='xxxx' user_id='xxxxxxxx' resource='pve_12' build_type='--release'
player_status prev_status='1' new_status='9' to='pve_12' // Bitfield 0:Online 1:??? 2:AFK 3:Lobby 4:Room 5:Playing 6:Shop 7:Inventory
profile_info_get_status nickname='xxxxxx'
iq/user_status[k01:presence] invalid

======== */