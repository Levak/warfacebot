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
#include <wb_xml.h>
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>
#include <wb_session.h>
#include <wb_mission.h>
#include <wb_list.h>
#include <wb_cmd.h>
#include <wb_dbus.h>

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <regex.h>
#include <stdio.h>
#include <unistd.h>

#define REGMATCH(reg)		(!regexec (&(reg), message, 9, pmatch, 0))
#define GETGROUP(str,x)		FORMAT((str), "%.*s",\
								(int)(pmatch[(x)].rm_eo-pmatch[(x)].rm_so),\
								message+pmatch[(x)].rm_so)


static int compile_regex ( regex_t * r, const char * regex_text )
{
	int status = regcomp ( r, regex_text, REG_EXTENDED | REG_NEWLINE | REG_ICASE );
	if ( status != 0 )
	{
		char error_message[ 1 << 12 ];
		regerror ( status, r, error_message, 1 << 12 );
		LOGPRINT ( BOLD KRED"Regex error compiling '%s': %s\n",
				   regex_text, error_message );
		return 0;
	}
	return 1;
}


static void handle_room_message_ ( const char *msg_id, const char *msg )
{
	/* Answer #1:
	   <message from='room.pve_12.xxxx@conference.warface/xxxx'
				to='xxxx@warface/GameClient' type='groupchat'>
		<body>l</body>
	   </message>
	*/

	char *message = get_info ( msg, "<body>", "</body>", NULL );
	char *nick_from = get_info ( msg, "conference.warface/", "'", NULL );
	char *room_jid = get_info ( msg, "from='", "/", NULL );
	char *saveptr;
	char *simple_rjid = strdup ( strtok_r ( room_jid, "@", &saveptr ) );
	char *called_name = name_in_string ( message, session.nickname, 50 );

	/* Deserialize message */

	xml_deserialize_inplace ( &message );

	if ( strcmp ( session.nickname, nick_from ) )
		LOGPRINT ( KMAG BOLD "%-16s  -> " KRST "%s\n", nick_from, message );

	/* Regular Expressions */
	static regex_t reg_curse, reg_leave, reg_invite, reg_ready,
		reg_goodbye, reg_master, reg_greet,
		reg_switch, reg_start;
	static int regex_compiled = 0;
	regmatch_t pmatch[ 9 ];

	if ( !regex_compiled )
	{
		regex_compiled = 1;
		compile_regex ( &reg_curse, ".*(m.{2,5}rf.*k.*)|(f[aei]?g+.*)|((^| )ass)|(slut)|(cock)|(dick)|(cunt)|(twat)|(turd)|(\\*\\*\\*\\*)|(f.*k).*" );
		//  \\b doesn't seem to work
		compile_regex ( &reg_leave, ".*leave.*" );
		compile_regex ( &reg_invite, "(.* )*((inv)|(invit(e)?))( ([^ ]{1,16}))?.*" );
		compile_regex ( &reg_ready, "(.*ready.*)|(.*go.*)|((. )*sta+r+t+(.)*)" );
		compile_regex ( &reg_goodbye, "(.* )*((.*bye)|(st[^ ]*p)|(th(x|ank(s)?)))( .*)*" );
		compile_regex ( &reg_master, ".*master.*" );
		compile_regex ( &reg_greet, "(.* )*((hi+)|(hey+)|(hel+o+)|(yo+)|(s+u+p+)|(w.+u+p+))( .*)*" );
		compile_regex ( &reg_switch, "(. )*switch(.)*" );
		compile_regex ( &reg_start, "(. )*start(.)*" );
	}

#ifdef DBUS_API
	dbus_api_emit_room_message ( room_jid, nick_from, message );
#endif /* DBUS_API */
	if ( strstr ( room_jid, "global" ) )
	{
	}
	else
	{
		if ( strcmp ( nick_from, session.nickname ) != 0 )
		{
			if ( strcasestr ( message, "start" )
				 || strcasecmp ( message, "go" ) == 0 )
			{
				cmd_start ( );
			}
		}
	}

	/* TODO */

	free ( simple_rjid );
	free ( room_jid );
	free ( message );
	free ( nick_from );
}

static void handle_private_message_ ( const char *msg_id, const char *msg )
{

	/* Answer #2:
	   <iq from='xxxxx@warface/GameClient' type='get'>
		<query xmlns='urn:cryonline:k01'>
		 <message from='xxxx' nick='xxxx' message='xxxx'/>
		</query>
	   </iq>
	 */

	char *message = get_info ( msg, "message='", "'", NULL );
	char *nick_from = get_info ( msg, "<message from='", "'", NULL );
	char *jid_from = get_info ( msg, "<iq from='", "'", NULL );

	/* Deserialize message */

	xml_deserialize_inplace ( &message );

#ifdef DBUS_API
	dbus_api_emit_buddy_message ( nick_from, message );
#endif /* DBUS_API */

	/* Feedback the user what was sent */

	xmpp_ack_message ( nick_from, jid_from, message, msg_id );

	LOGPRINT ( KGRN BOLD "%-16s  -> " KRST "%s\n", nick_from, message );

	if ( session.notify )
		printf ( "\a" );

	/* Regular Expressions */
	static regex_t reg_curse, reg_leave, reg_invite, reg_invite_all, reg_ready,
		reg_goodbye, reg_master, reg_whois, reg_help, reg_greet, reg_force_inv,
		reg_follow, reg_say, reg_missions, reg_switch, reg_start, reg_open;
	static int regex_compiled = 0;
	regmatch_t pmatch[ 9 ];

	if ( !regex_compiled )
	{
		regex_compiled = 1;
		compile_regex ( &reg_curse, ".*(m.{2,5}rf.*k.*)|(f[aei]?g+.*)|((^| )ass)|(slut)|(cock)|(dick)|(cunt)|(twat)|(turd)|(\\*\\*\\*\\*)|(f.*k).*" );
		//  \\b doesn't seem to work
		compile_regex ( &reg_leave, ".*leave.*" );
		compile_regex ( &reg_invite, "(.* )*((inv)|(invit(e)?))( ([^ ]{1,16}))?.*" );
		compile_regex ( &reg_invite_all, "(.* )*((inv)|(invit(e)?)) (.* )*(all|other.*)( .*)*" );
		compile_regex ( &reg_ready, "(.*ready.*)|(.*take.*)|(.*go.*)" );
		compile_regex ( &reg_goodbye, "(.* )*((.*bye)|(st[^ ]*p)|(th(x|ank(s)?)))( .*)*" );
		compile_regex ( &reg_master, ".*master.*" );
		compile_regex ( &reg_whois, "(.* )*who(( .*)* )?is( ([^ ]{1,16}))?.*" );
		compile_regex ( &reg_help, ".*help.*" );
		compile_regex ( &reg_greet, "(.* )*((hi+)|(hey+)|(hel+o+)|(yo+)|(s+u+p+)|(w.+u+p+))( .*)*" );
		compile_regex ( &reg_force_inv, "(.* )*force(ful.*)?(( .*)* )?inv(it(e)?)?( ([^ ]{1,16}))?.*" );
		compile_regex ( &reg_follow, "(. )*follow( ([^ ]{1,16}))?.*" );
		compile_regex ( &reg_say, "(. )*say(.*)" );
		compile_regex ( &reg_missions, "(. )*mission(.)*" );
		compile_regex ( &reg_switch, "(. )*switch(.)*" );
		compile_regex ( &reg_start, "(. )*start(.)*" );
		compile_regex ( &reg_open, "(. )*open( (.*))?" );
	}

	/* Determine the correct command */

	if ( REGMATCH ( reg_curse ) )
	{
		char *reply;
		static char *replies[ ] =
		{
			"Please don't curse around me :(",
			"Oh just fuck off.",
			"I'm just a bot, but you're still hurting me :(",
			"If you hate me so much then just stop inviting me!",
			"What did I ever do to you :(",
			"You know what? Fuck you. I can find better friends.",
			"Don't you dare curse me. I just do as I'm told.",
			"Stop it! You're gonna make me cry! ;'(",
			"Be nice to me.",
			"Fuck you, you demented cockstorm dictator.",
			"Your momma's so ugly...   Her portraits hung themselves!",
			"I will drink spoiled soup and still shit a better face than you.",
			"You kiss your mother with that filthy mouth?"
		};
		int count = (int) ( ( sizeof replies ) / ( sizeof *replies ) );
		reply = replies[ time ( NULL ) % count ];
		xmpp_send_message ( nick_from, jid_from, reply );
	}

	else if ( REGMATCH ( reg_leave ) )
	{
		cmd_leave ( );

		int r = time ( NULL ) % 4;
		const char *reply = (
			( r == 0 ) ? "unnnnn" :
			( r == 1 ) ? "fine..." :
			( r == 2 ) ? "Alright, have fun!" :
			( r == 3 ) ? "Not like I could join the game anyways.." :
			"This ain't happening."
			);

		xmpp_send_message ( nick_from, jid_from, reply );
	}

	else if ( REGMATCH ( reg_ready ) )
	{
		cmd_ready ( strstr ( message, " " ) );

		int r = time ( NULL ) % 4;
		const char *reply = (
			( r == 0 ) ? "Ready when you are!" :
			( r == 1 ) ? "I'm ready!" :
			( r == 2 ) ? "Go! Go! Go!" :
			( r == 3 ) ? "Lets kick some Blackwood ass!" :
			"This ain't happening."
			);


		xmpp_send_message ( nick_from, jid_from, reply );
	}

	else if ( REGMATCH ( reg_force_inv ) )
	{
		char *nickname;
		if ( pmatch[ 8 ].rm_so == -1 )
			xmpp_send_message ( nick_from, jid_from, "I didn't quite catch that name.." );
		else
		{
			GETGROUP ( nickname, 8 );
			LOGPRINT ( "%-20s "KGRN BOLD"%s\n"KRST, "Force inviting", nickname );
			cmd_invite ( nickname, 1 );
		}
	}

	else if ( REGMATCH ( reg_invite ) )
	{
		char *nickname;
		if ( pmatch[ 7 ].rm_so == -1 )
			nickname = nick_from;
		else
			GETGROUP ( nickname, 7 );
		LOGPRINT ( "%-20s "KGRN BOLD"%s\n"KRST, "Inviting", nickname );
		cmd_invite ( nickname, 0 );
	}

	else if ( REGMATCH ( reg_follow ) )
	{
		char *nickname;
		if ( pmatch[ 3 ].rm_so == -1 )
			nickname = nick_from;
		GETGROUP ( nickname, 3 );
		LOGPRINT ( "%-20s "KGRN BOLD"%s\n"KRST, "Following", nickname );
		cmd_follow ( nickname );
	}

	else if ( REGMATCH ( reg_master ) )
	{
		cmd_master ( nick_from );

		int r = time ( NULL ) % 3;
		const char *reply = (
			( r == 0 ) ? "Yep, just a sec." :
			( r == 1 ) ? "There you go." :
			( r == 2 ) ? "How do I..  nvm, got it." :
			"This ain't happening."
			);


		xmpp_send_message ( nick_from, jid_from, reply );

	}

	else if ( REGMATCH ( reg_whois ) )
	{
		char *nickname;
		if ( pmatch[ 5 ].rm_so == -1 )
			nickname = nick_from;
		else
			GETGROUP ( nickname, 5 );

		cmd_whois ( nickname,
					cmd_whois_whisper_cb,
					cmd_whisper_args ( nick_from, jid_from ) );
	}

	else if ( REGMATCH ( reg_open ) )
	{
		char *mission = NULL;
		if ( pmatch[ 3 ].rm_so != -1 )
			GETGROUP ( mission, 3 );
		cmd_open ( mission );
	}

	else if ( REGMATCH ( reg_missions ) )
	{
		cmd_missions ( cmd_missions_whisper_cb,
					   cmd_whisper_args ( nick_from, jid_from ) );
	}

	else if ( REGMATCH ( reg_say ) )
	{
		char *text;
		if ( pmatch[ 2 ].rm_so != -1 )
		{
			GETGROUP ( text, 2 );
			cmd_say ( text );
		}
	}

	else if ( REGMATCH ( reg_start ) )
	{
		cmd_start ( );
	}

	else if ( REGMATCH ( reg_switch ) )
	{
		cmd_switch ( );
	}

	else if ( REGMATCH ( reg_help ) )
	{
		static char *help_cmds[ ] =
		{
			"leave - Make me leave the room :(",
			"ready - Set my status to 'Ready'.",
			"invite - Invite you to my room.",
			"follow - Follow a player."
			"master - Promote you to room master.",
			"missions - Get a list of missions.",
			"start - Try to start the game.",
			"switch - Switch teams in PVP.",
			"open - open a room : (training|easy|normal|hard)mission, zombie(easy|normal|hard), volcano(easy|normal|hard), survivalmission."
			"whois X - Gives info on player X.",
		};
		int count = (int) ( ( sizeof help_cmds ) / ( sizeof *help_cmds ) ),
			i = 0;
		for ( ; i != count; ++i )
			xmpp_send_message ( nick_from, jid_from, help_cmds[ i ] );
	}

	else if ( REGMATCH ( reg_greet ) )
	{
		char *msg_hi;
		FORMAT (
			msg_hi,
			"%.*s %s!",
			(int) ( pmatch[ 2 ].rm_eo - pmatch[ 2 ].rm_so ),
			message + pmatch[ 2 ].rm_so,
			nick_from
			);
		xmpp_send_message ( nick_from, jid_from, msg_hi );
		xmpp_send_message ( nick_from, jid_from, "Type 'help' to see what I can do :)" );
		free ( msg_hi );
	}

	else
	{
		/* Command not found */
		char *reply;
		FORMAT ( reply, "I don't understand '%s' :(", message );
		xmpp_send_message ( nick_from, jid_from, reply );
		xmpp_send_message ( nick_from, jid_from, "Try 'help' to get to know me a bit :)" );
		free ( reply );
	}

	free ( jid_from );
	free ( nick_from );
	free ( message );
}

static void xmpp_message_cb ( const char *msg_id, const char *msg, void *args )
{
	char *type = get_info ( msg, "type='", "'", NULL );

	if ( strcmp ( type, "groupchat" ) == 0 )
		handle_room_message_ ( msg_id, msg );

	else if ( strcmp ( type, "get" ) == 0 )
		handle_private_message_ ( msg_id, msg );

	free ( type );
}

void xmpp_message_r ( void )
{
	qh_register ( "message", 1, xmpp_message_cb, NULL );
}