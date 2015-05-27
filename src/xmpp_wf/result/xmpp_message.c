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

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <regex.h>
#include <stdio.h>

static int compile_regex (regex_t * r, const char * regex_text)
{
    int status = regcomp (r, regex_text, REG_EXTENDED|REG_NEWLINE|REG_ICASE);
    if (status != 0) {
	char error_message[1<<12];
	regerror (status, r, error_message, 1<<12);
        printf ("Regex error compiling '%s': %s\n",
                 regex_text, error_message);
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
		printf ( "Inviting:   %s\n", nick );
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
        puts ( f->nickname );
        puts ( msg->msg );
        xmpp_send_message(session.wfs, session.nickname, session.jid,
                              f->nickname, f->jid,
                              msg->msg, NULL);
    }
}

static void handle_room_message_(const char *msg_id, const char *msg)
{
    /* Answer #1:
       <message from='room.pve_12.xxxx@conference.warface/xxxx'
                to='xxxx@warface/GameClient' type='groupchat'>
        <body>l</body>
       </message>
    */

    /* TODO */
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
			reg_greet;
	static int regex_compiled = 0;
	regmatch_t pmatch[9];
	if ( !regex_compiled )
	{
		puts("Compiling regex.");
		regex_compiled = 1;
		regex_compiled &= compile_regex ( &reg_curse, ".*(m.{2,5}rf.*k.*)|(f[aei]?g+.*)|(ass)|(slut)|(cock)|(dick)|(cunt)|(twat)|(turd)|(\\*\\*\\*\\*)|(f.*k).*" );
													//  \\b doesn't seem to work
		regex_compiled &= compile_regex ( &reg_leave, ".*leave.*" );
		regex_compiled &= compile_regex ( &reg_send, "send (to )?([^ ]{1,16}) (.*)" );
		regex_compiled &= compile_regex ( &reg_list_all, "(.* )*list (.* )*all( .*)*" );
		regex_compiled &= compile_regex ( &reg_list_online, "(.* )*list (.* )*online( .*)*" );
		regex_compiled &= compile_regex ( &reg_invite_all, "(.* )*((inv)|(invit(e)?)) (.* )*all( .*)*" );
		regex_compiled &= compile_regex ( &reg_ready, ".*ready.*" );
		regex_compiled &= compile_regex ( &reg_invite, "(.* )*((inv)|(invit(e)?))( .*)*" );
		regex_compiled &= compile_regex ( &reg_master, ".*master.*" );
		regex_compiled &= compile_regex ( &reg_whois, "(.* )*who(( .*)* )?is( ([^ ]{1,16}))?.*" );
		regex_compiled &= compile_regex ( &reg_help, ".*help.*" );
		regex_compiled &= compile_regex ( &reg_greet, "(.* )*((h+i+)|(hey+)|(hel+o+)|(yo+)|(s+u+p+)|(w.+u+p+))( .*)*" );
		if ( !regex_compiled )
			puts("Failed to compiled some regex.");
	}

    /* Feedback the user what was sent */

    xmpp_send_message(session.wfs, nick_from, session.jid,
                      session.nickname, jid_from,
                      message, msg_id);

    /* Determine the correct command */

	
	printf ( "%s:\t%s\n", nick_from, message );
	
#define WHISPER(x)			xmpp_send_message(session.wfs, session.nickname, session.jid,\
								nick_from, jid_from,\
								(x), NULL)
#define REGMATCH(reg)		(!regexec (&(reg), message, 9, pmatch, 0))
#define GETGROUP(str,x)		sprintf((str), "%.*s",\
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
            msg_info.nick = malloc(32),
            msg_info.msg = malloc(1<<8);
			GETGROUP(msg_info.nick, 1);
			GETGROUP(msg_info.msg, 2);
			puts(msg_info.nick);
			puts(msg_info.msg);
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
						"This ain&apos;t happening."
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
						(r == 1) ? "I&apos;m ready!" :
						(r == 2) ? "Go! Go! Go!" :
						(r == 3) ? "Lets kick some Blackwood ass!" :
						"This ain&apos;t happening."
					);
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
						"This ain&apos;t happening."
					);

		}

		else if (REGMATCH(reg_whois))
		{
			char *nickname = malloc(18);
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
				"ready - Set my status to &apos;Ready&apos;.",
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
			char *msg_hi = malloc ( strlen(message) + 20 );
			sprintf(
						msg_hi,
						"%.*s %s!",
						(int)(pmatch[2].rm_eo-pmatch[2].rm_so),
						message+pmatch[2].rm_so,
						nick_from
					);
			WHISPER(msg_hi);
			WHISPER("Type &apos;help&apos; for a list of available commands.");
			free ( msg_hi );
		}

		else
		{
			/* Command not found */
			char *reply = malloc ( 256 );
			strcpy( reply, "I don&apos;t recognize &apos;" );
			strcat ( reply, message );
			strcat ( reply, "&apos; as a valid command." );
			WHISPER(reply);
			WHISPER("Try &apos;help&apos; to get a list of available commands.");
			free ( reply );
		}
	}
	else
	{
		char *reply;
		static char *replies[] = 
		{
			"Please don&apos;t curse around me :(",
			"Oh just fuck off.",
			"I&apos;m just a bot, but you&apos;re still hurting me :(",
			"If you hate me so much then just stop inviting me!",
			"What did I ever do to you :(",
			"You know what? Fuck you. I can find better friends.",
			"Don&apos;t you dare curse me. I just do as I&apos;m told.",
			"Stop it! You&apos;re gonna make me cry! ;&apos;(",
			"Be nice to me.",
			"Fuck you, you demented cockstorm dictator."
		};
		int count = (int)((sizeof replies ) / (sizeof *replies));
		reply = replies[time(NULL) % count];
		puts ( reply );
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
