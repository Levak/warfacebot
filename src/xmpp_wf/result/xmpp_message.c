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
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>
#include <wb_session.h>
#include <wb_mission.h>
#include <wb_list.h>
#include <listener.h>
#include <wb_xml.h>

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <regex.h>
#include <stdio.h>
#include <unistd.h>

static int compile_regex (regex_t * r, const char * regex_text)
{
    int status = regcomp (r, regex_text, REG_EXTENDED|REG_NEWLINE|REG_ICASE);
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

typedef struct
{
	char *nick_from, *jid_from;
}cmd_data_t;
cmd_data_t cmd_data;

static void all_my_friends_cb(void *friend, void *null)
{	
    struct friend *f = (struct friend*)friend;
    char *msg =         malloc(1<<7);
    sprintf(msg, "%s is %s", f->nickname,
            f->status & STATUS_AFK ? "AFK" :
            f->status & STATUS_PLAYING ? "playing" :
            f->status & STATUS_SHOP ? "in shop" :
            f->status & STATUS_INVENTORY ? "in inventory" :
            f->status & STATUS_ROOM ? "in a room" :
            f->status & STATUS_LOBBY ? "in lobby" :
            "offline"
           );
	xmpp_send_message(
						session.wfs, session.nickname, session.jid,
						cmd_data.nick_from, cmd_data.jid_from,
						msg, NULL
					    );
    free(msg);
}

static void my_online_friends_cb(void *friend, void *null)
{	
    struct friend *f = (struct friend*)friend;
	if ( f->status > 2 || f->status == 1 )
		xmpp_send_message(
							session.wfs, session.nickname, session.jid,
							cmd_data.nick_from, cmd_data.jid_from,
							f->nickname, NULL
						);
}

static void invite_online_friends_cb(void *friend, void *null)
{	
	int status = ((struct friend*)friend)->status;
	char *nick = strdup ( ((struct friend*)friend)->nickname );
	strcpy ( nick, ((struct friend*)friend)->nickname );
	if ( status > 2 || status == 1 )
	{
		LOGPRINT ( "%-16s "KCYN"%s\n"KRST, "Inviting", nick );
		send_stream_format(session.wfs,
							   "<iq to='masterserver@warface/%s' type='get'>"
							   " <query xmlns='urn:cryonline:k01'>"
							   "  <invitation_send nickname='%s' is_follow='0'/>"
							   " </query>"
							   "</iq>",
							   session.channel, nick);
	}
}

void list_iterate(int type)
{
    type == 1 ? list_foreach(session.friends, &all_my_friends_cb, NULL) :
	type == 2 ? list_foreach(session.friends, &my_online_friends_cb, NULL) :
	type == 3 ? list_foreach(session.friends, &invite_online_friends_cb, NULL) :
	NULL;
}  

typedef struct
{
    char *nick, *msg;
}message_t;

static void send_to_cb(void *friend, void *info)
{
    struct friend *f = (struct friend*)friend;
    message_t *msg = (message_t*) info;
    if ( !strcasecmp(msg->nick, f->nickname) )
    {
        xmpp_send_message(session.wfs, session.nickname, session.jid,
                              f->nickname, f->jid,
                              msg->msg, NULL);
    }
}

struct active_listeners_t *plisteners = NULL;

static void handle_room_message_(const char *msg_id, const char *msg)
{
    /* Answer #1:
       <message from='room.pve_12.xxxx@conference.warface/xxxx'
                to='xxxx@warface/GameClient' type='groupchat'>
        <body>l</body>
       </message>
    */

	if ( !plisteners )
		create_listeners ( );
	char *room_jid = get_info(msg, "from='", "/", NULL);
	char *nick_from = get_info(msg, "warface/", "'", NULL);
	if(!strcmp(nick_from, session.nickname))
	{
		free(room_jid);
		free(nick_from);
		return;
	}
	static regex_t reg_curse, reg_leave, reg_invite_all, reg_ready,
		reg_goodbye, reg_master, reg_whois, reg_help, reg_greet, reg_force_inv;
	static int regex_compiled = 0;
	regmatch_t pmatch[ 9 ];
	if ( !regex_compiled )
	{
		regex_compiled = 1;
		compile_regex ( &reg_curse, ".*(m.{2,5}rf.*k.*)|(f[aei]?g+.*)|((^| )ass)|(slut)|(cock)|(dick)|(cunt)|(twat)|(turd)|(\\*\\*\\*\\*)|(f.*k).*" );
		//  \\b doesn't seem to work
		compile_regex ( &reg_leave, ".*leave.*" );
		compile_regex ( &reg_invite_all, "(.* )*((inv)|(invit(e)?)) (.* )*(all|other.*)( .*)*" );
		compile_regex ( &reg_ready, ".*ready.*" );
		compile_regex ( &reg_goodbye, "(.* )*((.*bye)|(st[^ ]*p)|(th(x|ank(s)?)))( .*)*" );
		compile_regex ( &reg_master, ".*master.*" );
		compile_regex ( &reg_whois, "(.* )*who(( .*)* )?is( ([^ ]{1,16}))?.*" );
		compile_regex ( &reg_help, ".*help.*" );
		compile_regex ( &reg_greet, "(.* )*((hi+)|(hey+)|(hel+o+)|(yo+)|(s+u+p+)|(w.+u+p+))( .*)*" );
		compile_regex ( &reg_force_inv, "(.* )*force(ful.*)?(( .*)* )?inv(it(e)?)?( ([^ ]{1,16}))?.*" );
	}
#define REGMATCH(reg)		(!regexec (&(reg), message, 9, pmatch, 0))
#define GETGROUP(str,x)		FORMAT((str), "%.*s",\
								(int)(pmatch[(x)].rm_eo-pmatch[(x)].rm_so),\
								message+pmatch[(x)].rm_so)
#define SAYINROOM(x)		do {\
								xmpp_send_message_room( session.wfs, session.nickname,\
								room_jid, (x));\
								sleep(3);\
							} while(0)
	char *message = get_info(msg, "<body>", "</body>", NULL);
	xml_deserialize_inplace ( &message );
	LOGPRINT ( KYEL"%-16s "KGRN"%s\n"KRST, nick_from, message );
	if ( name_in_string(message, session.nickname, 50) )
	{
		char *reply = NULL;
		if ( REGMATCH ( reg_goodbye ) )
		{
			if ( !is_active_listener( nick_from ) )
				FORMAT ( reply, "I wasn't even talking to you, %s.", nick_from );
		}
		else
		{
			char *greeting;
			if ( REGMATCH ( reg_greet ) )
				GETGROUP ( greeting, 2 );
			else
				greeting = strdup ( "Hi" );
			char *old = add_listener ( nick_from );
			if ( !old )
				FORMAT ( reply, "%s %s!", greeting, nick_from );
			else if ( strcmp ( old, nick_from ) )
					FORMAT ( reply, "Replacing %s with %s.", old, nick_from );
			free ( old );
			free ( greeting );
		}
		if ( reply )
		{
			SAYINROOM ( reply );
			free ( reply );
		}
	}
	if ( !is_active_listener ( nick_from ) )
	{
		free ( message );
		free ( nick_from );
		free ( room_jid );
		return;
	}
	if ( !REGMATCH ( reg_curse ) )
	{

		if ( REGMATCH ( reg_leave ) )
		{
			xmpp_iq_gameroom_leave ( );

			int r = time ( NULL ) % 4;
			SAYINROOM (
				( r == 0 ) ? "unnnnn" :
				( r == 1 ) ? "fine..." :
				( r == 2 ) ? "Alright, have fun!" :
				( r == 3 ) ? "Not like I could join the game anyways.." :
				"This ain't happening."
				);
		}

		else if ( REGMATCH ( reg_invite_all ) )
		{
			list_iterate ( 3 );
		}

		else if ( REGMATCH ( reg_ready ) )
		{
			send_stream_format ( session.wfs,
								 "<iq to='masterserver@warface/%s' type='get'>"
								 " <query xmlns='urn:cryonline:k01'>"
								 "  <gameroom_setplayer team_id='0' status='1' class_id='0'/>"
								 " </query>"
								 "</iq>",
								 session.channel );

			int r = time ( NULL ) % 4;
			SAYINROOM (
				( r == 0 ) ? "Ready when you are!" :
				( r == 1 ) ? "I'm ready!" :
				( r == 2 ) ? "Go! Go! Go!" :
				( r == 3 ) ? "Lets kick some Blackwood ass!" :
				"This ain't happening."
				);
		}

		else if ( REGMATCH ( reg_master ) )
		{
			xmpp_promote_room_master ( nick_from );

			int r = time ( NULL ) % 3;
			SAYINROOM (
				( r == 0 ) ? "Yep, just a sec." :
				( r == 1 ) ? "There you go." :
				( r == 2 ) ? "How do I..  nvm, got it." :
				"This ain't happening."
				);

		}

		else if ( REGMATCH ( reg_help ) )
		{
			SAYINROOM ( "Leave, Ready, Invite, Master, Invite all." );
		}

		else if ( REGMATCH ( reg_force_inv ) )
		{
			char *nickname;
			if ( pmatch[ 8 ].rm_so == -1 )
				SAYINROOM ( "I didn't quite catch that name.." );
			else
			{
				GETGROUP ( nickname, 8 );
				LOGPRINT ( "%-16s "KGRN BOLD"%s\n"KRST, "Force inviting", nickname );
				send_stream_format ( session.wfs,
									 "<iq to='masterserver@warface/%s' type='get'>"
									 " <query xmlns='urn:cryonline:k01'>"
									 "  <invitation_send nickname='%s' is_follow='2'/>"
									 " </query>"
									 "</iq>",
									 session.channel, nickname );
			}
		}

		else if ( REGMATCH ( reg_goodbye ) )
		{
			char *reply;
			remove_listener ( nick_from );
			FORMAT ( reply, "Happy to help you, %s. :)", nick_from );
			SAYINROOM ( reply );
			free ( reply );
		}
	}
	else
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
			"Fuck you, you demented cockstorm dictator."
		};
		int count = (int) ( ( sizeof replies ) / ( sizeof *replies ) );
		reply = replies[ time ( NULL ) % count ];
		SAYINROOM ( reply );
	}

	free ( message );
	free ( nick_from );
	free ( room_jid );
#undef GETGROUP
#undef SAYINROOM
#undef REGMATCH
}

static void handle_private_message_(const char *msg_id, const char *msg)
{

    /* Answer #2:
       <iq from='xxxxx@warface/GameClient' type='get'>
        <query xmlns='urn:cryonline:k01'>
         <message from='xxxx' nick='xxxx' message='xxxx'/>
        </query>
       </iq>
     */

    char *message = get_info(msg, "message='", "'", NULL);
    char *nick_from = get_info(msg, "<message from='", "'", NULL);
    char *jid_from = get_info(msg, "<iq from='", "'", NULL);
	static regex_t reg_curse, reg_leave, reg_send, reg_list_all, reg_list_online,
			reg_invite_all, reg_ready, reg_invite, reg_master, reg_whois, reg_help,
			reg_greet, reg_force_inv;
	static int regex_compiled = 0;
	regmatch_t pmatch[9];
	if ( !regex_compiled )
	{
		regex_compiled = 1;
		compile_regex ( &reg_curse, ".*(m.{2,5}rf.*k.*)|(f[aei]?g+.*)|((^| )ass)|(slut)|(cock)|(dick)|(cunt)|(twat)|(turd)|(\\*\\*\\*\\*)|(f([^ ]*)k).*" );
					//  \\b doesn't seem to work
		compile_regex ( &reg_leave, ".*leave.*" );
		compile_regex ( &reg_send, "send (to )?([^ ]{1,16}) (.*)" );
		compile_regex ( &reg_list_all, "(.* )*list (.* )*all( .*)*" );
		compile_regex ( &reg_list_online, "(.* )*list (.* )*online( .*)*" );
		compile_regex ( &reg_invite_all, "(.* )*((inv)|(invit(e)?)) (.* )*all( .*)*" );
		compile_regex ( &reg_ready, ".*ready.*" );
		compile_regex ( &reg_invite, "(.* )*((inv)|(invit(e)?))( .*)*" );
		compile_regex ( &reg_master, ".*master.*" );
		compile_regex ( &reg_whois, "(.* )*who(( .*)* )?is( ([^ ]{1,16}))?.*" );
		compile_regex ( &reg_help, ".*help.*" );
		compile_regex ( &reg_greet, "(.* )*((hi+)|(hey+)|(hel+o+)|(yo+)|(s+u+p+)|(w.+u+p+))( .*)*" );
		compile_regex ( &reg_force_inv, "(.* )*force(ful.*)?(( .*)* )?inv(it(e)?)?( ([^ ]{1,16}))?.*" );
	}

    /* Feedback the user what was sent */

    xmpp_send_message(session.wfs, nick_from, session.jid,
                      session.nickname, jid_from,
                      message, msg_id);

    /* Determine the correct command */
	xml_deserialize_inplace ( &message );
	LOGPRINT (  KYEL"%-16s "KCYN"%s\n"KRST, nick_from, message );
#define WHISPER(x)			xmpp_send_message(session.wfs, session.nickname, session.jid,\
								nick_from, jid_from,\
								(x), NULL)
#define REGMATCH(reg)		(!regexec (&(reg), message, 9, pmatch, 0))
#define GETGROUP(str,x)		FORMAT((str), "%.*s",\
								(int)(pmatch[(x)].rm_eo-pmatch[(x)].rm_so),\
								message+pmatch[(x)].rm_so)
	
	// To print all matches
	// int i;
	// for ( i = 1 ; i != 9; ++i )
		// if (pmatch[i].rm_so != -1)
			// printf("%d:\t%.*s\n", i, (int)(pmatch[i].rm_eo-pmatch[i].rm_so), message+pmatch[i].rm_so);
		
	if (!REGMATCH (reg_curse))
	{
		if (REGMATCH(reg_send))
		{
            message_t msg_info;
			GETGROUP(msg_info.nick, 2);
			GETGROUP(msg_info.msg, 3);
			list_foreach(session.friends, &send_to_cb, &msg_info );
            free(msg_info.nick);
            free(msg_info.msg);
		}
		
		else if (REGMATCH(reg_leave))
		{
			xmpp_iq_gameroom_leave();

			int r = time(NULL) % 4;
			WHISPER(
						(r == 0) ? "unnnnn" :
						(r == 1) ? "fine..." :
						(r == 2) ? "Alright, have fun!" :
						(r == 3) ? "Not like I could join the game anyways.." :
						"This ain't happening."
					);
		}
		
		else if (REGMATCH(reg_list_online))
		{
			cmd_data.nick_from = strdup(nick_from);
			cmd_data.jid_from = strdup(jid_from);
			list_iterate(2);
			free(cmd_data.nick_from);
			free(cmd_data.jid_from);
		}
	
		else if (REGMATCH(reg_list_all))
		{
			cmd_data.nick_from = strdup(nick_from);
			cmd_data.jid_from = strdup(jid_from);
			list_iterate(1);
			free(cmd_data.nick_from);
			free(cmd_data.jid_from);
		}
		
		else if (REGMATCH(reg_invite_all))
		{
			cmd_data.nick_from = strdup(nick_from);
			cmd_data.jid_from = strdup(jid_from);
			list_iterate(3);
			free(cmd_data.nick_from);
			free(cmd_data.jid_from);
		}

		else if (REGMATCH(reg_ready))
		{
			send_stream_format(session.wfs,
							   "<iq to='masterserver@warface/%s' type='get'>"
							   " <query xmlns='urn:cryonline:k01'>"
							   "  <gameroom_setplayer team_id='0' status='1' class_id='0'/>"
							   " </query>"
							   "</iq>",
							   session.channel);

			int r = time(NULL) % 4;
			WHISPER(
						(r == 0) ? "Ready when you are!" :
						(r == 1) ? "I'm ready!" :
						(r == 2) ? "Go! Go! Go!" :
						(r == 3) ? "Lets kick some Blackwood ass!" :
						"This ain't happening."
					);
		}

		else if (REGMATCH(reg_force_inv))
		{
			char *nickname;
			if ( pmatch[ 8 ].rm_so == -1 )
				WHISPER ( "I didn't quite catch that name.." );
			else
			{
				GETGROUP ( nickname, 8 );
				LOGPRINT ( "%-16s "KGRN BOLD"%s\n"KRST, "Force inviting", nickname );
				send_stream_format ( session.wfs,
									 "<iq to='masterserver@warface/%s' type='get'>"
									 " <query xmlns='urn:cryonline:k01'>"
									 "  <invitation_send nickname='%s' is_follow='2'/>"
									 " </query>"
									 "</iq>",
									 session.channel, nickname );
			}
		}

		else if (REGMATCH(reg_invite))
		{
			send_stream_format(session.wfs,
							   "<iq to='masterserver@warface/%s' type='get'>"
							   " <query xmlns='urn:cryonline:k01'>"
							   "  <invitation_send nickname='%s' is_follow='0'/>"
							   " </query>"
							   "</iq>",
							   session.channel, nick_from);
		}

		else if (REGMATCH(reg_master))
		{
			xmpp_promote_room_master(nick_from);

			int r = time(NULL) % 3;
			WHISPER(
						(r == 0) ? "Yep, just a sec." :
						(r == 1) ? "There you go." :
						(r == 2) ? "How do I..  nvm, got it." :
						"This ain't happening."
					);

		}

		else if (REGMATCH(reg_whois))
		{
			char *nickname;
			if (pmatch[5].rm_so == -1 )
				nickname = nick_from;
			else
				GETGROUP(nickname, 5);

			xmpp_iq_profile_info_get_status(nickname, nick_from, jid_from);
			
		}

		else if (REGMATCH(reg_help))
		{
			static char *help_cmds[] =
			{
				"leave - Make me leave the room :(",
				"ready - Set my status to 'Ready'.",
				"invite - Invite you to my room.",
				"master - Promote you to room master.",
				"whois X - Gives info on player X",
				"list all - See my friend-list.",
				"list online - See all my online friends.",
				"invite online - Invite all my friends."
			};
			int count = (int)((sizeof help_cmds ) / (sizeof *help_cmds)),
				i = 0;
			for ( ; i != count; ++i )
				WHISPER(help_cmds[i]);
		}

		else if (strstr(message, "missions"))
		{
			struct cb_args
			{
				char *nick_from;
				char *jid_from;
			};

			void cbm(struct mission *m, void *args)
			{
				struct cb_args *a = (struct cb_args *) args;
				char *answer;
				FORMAT(answer, "mission %s %i %i",
					   m->type,
					   m->crown_time_gold,
					   m->crown_perf_gold);

				xmpp_send_message(session.wfs, session.nickname, session.jid,
								  a->nick_from, a->jid_from,
								  answer, NULL);

				free(answer);
			}

			void cb(struct list *l, void *args)
			{
				struct cb_args *a = (struct cb_args *) args;

				list_foreach(l, (f_list_callback) cbm, args);

				list_free(l);
				free(a->jid_from);
				free(a->nick_from);
				free(a);
			}

			struct cb_args *a = calloc(1, sizeof (struct cb_args));
			a->nick_from = strdup(nick_from);
			a->jid_from = strdup(jid_from);
			xmpp_iq_missions_get_list(cb, a);
		}

		else if (REGMATCH(reg_greet))
		{
			char *msg_hi;
			FORMAT(
						msg_hi,
						"%.*s %s!",
						(int)(pmatch[2].rm_eo-pmatch[2].rm_so),
						message+pmatch[2].rm_so,
						nick_from
					);
			WHISPER ( msg_hi );
			WHISPER ( "Type 'help' for a list of available commands." );
			free ( msg_hi );
		}

		else
		{
			/* Command not found */
			char *reply = malloc ( 256 );
			strcpy( reply, "I don't recognize '" );
			strcat ( reply, message );
			strcat ( reply, "' as a valid command." );
			WHISPER ( reply );
			WHISPER ( "Try 'help' to get a list of available commands." );
			free ( reply );
		}
	}
	else
	{
		char *reply;
		static char *replies[] = 
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
			"Fuck you, you demented cockstorm dictator."
		};
		int count = (int)((sizeof replies ) / (sizeof *replies));
		reply = replies[time(NULL) % count];
		WHISPER(reply);
	}
    free(jid_from);
    free(nick_from);
    free(message);
}
	
#undef GETGROUP
#undef REGMATCH

static void xmpp_message_cb(const char *msg_id, const char *msg, void *args)
{
    if (xmpp_is_error(msg))
        return;

    char *type = get_info(msg, "type='", "'", NULL);

    if (strcmp(type, "result") == 0)
        ;

    else if (strcmp(type, "groupchat") == 0)
        handle_room_message_(msg_id, msg);

    else if (strcmp(type, "get") == 0)
        handle_private_message_(msg_id, msg);

    free(type);
}

void xmpp_message_r(void)
{
    qh_register("message", xmpp_message_cb, NULL);
}
