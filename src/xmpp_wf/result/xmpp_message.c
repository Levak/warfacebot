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

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <regex.h>
#include <stdio.h>

regex_t curse;
int curse_compiled = 0;

static int compile_regex (regex_t * r, const char * regex_text)
{
    int status = regcomp (r, regex_text, REG_EXTENDED|REG_NEWLINE);
    if (status != 0) {
	char error_message[1<<12];
	regerror (status, r, error_message, 1<<12);
        printf ("Regex error compiling '%s': %s\n",
                 regex_text, error_message);
        return 1;
    }
    return 0;
}

typedef struct
{
	struct session *session;
	char *nick_from, *jid_from;
}cmd_data_t;
cmd_data_t cmd_data;

static void all_my_friends_cb(void *friend)
{	
	xmpp_send_message(
						cmd_data.session->wfs, cmd_data.session->nickname, cmd_data.session->jid,
						cmd_data.nick_from, cmd_data.jid_from,
						((struct friend*)friend)->nickname, NULL
					 );
}

static void my_online_friends_cb(void *friend)
{	
	int status = ((struct friend*)friend)->status;
	if ( status > 2 || status == 1 )
		xmpp_send_message(
							cmd_data.session->wfs, cmd_data.session->nickname, cmd_data.session->jid,
							cmd_data.nick_from, cmd_data.jid_from,
							((struct friend*)friend)->nickname, NULL
						);
}

static void invite_online_friends_cb(void *friend)
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
							   cmd_data.session->channel, nick);
	}
}

void list_iterate(int type)
{
    type == 1 ? list_foreach(session.friends, &all_my_friends_cb) :
	type == 2 ? list_foreach(session.friends, &my_online_friends_cb) :
	type == 3 ? list_foreach(session.friends, &invite_online_friends_cb) :
	NULL;
}  

static void xmpp_message_cb(const char *msg_id, const char *msg)
{
    if (strstr(msg, "type='result'"))
        return;

    char *message = get_info(msg, "message='", "'", NULL);
    char *id = get_info(msg, "id='", "'", NULL);
    char *nick_from = get_info(msg, "<message from='", "'", NULL);
    char *jid_from = get_info(msg, "<iq from='", "'", NULL);
	if ( !curse_compiled )
		curse_compiled = compile_regex ( &curse, "(.*(((you)|(yu)|(u)) )?((m.{2,5}rf.*k.*)|(f[aei]?g.*t)|(ass)|(cock)|(dick)|(cunt)|(twat)|(turd)|(\\*\\*\\*\\*)|(f.*k)).*)" );
													// ^^ is redundant now..            \\b doesn't seem to work
    /* 1. Feedback the user what was sent (SERIOUSLY ? CRYTEK ? XMPP 4 DUMMIES ?) */

    xmpp_send_message(session.wfs, nick_from, session.jid,
                      session.nickname, jid_from,
                      message, id);

    /* */
	
	printf ( "%s:\t%s\n", nick_from, message );
	
#define MESSAGE(x)		!strcasecmp(message, (x))
#define MESSAGEA(x)		!strncmp(message, (x), strlen((x)))
	
	if ( regexec ( &curse, message, 0, NULL, 0 ) == REG_NOMATCH )
	{
		if (MESSAGE("leave"))
		{
			xmpp_iq_gameroom_leave();

			xmpp_send_message(session.wfs, session.nickname, session.jid,
							  nick_from, jid_from,
							  "but whyy :(", NULL);
		}
		
		else if (MESSAGE("list all"))
		{
			cmd_data.session = &session;
			cmd_data.nick_from = malloc ( 64 );
			cmd_data.jid_from = malloc ( 64 );
			strcpy(cmd_data.nick_from, nick_from);
			strcpy(cmd_data.jid_from, jid_from);
			list_iterate(1);
			free(cmd_data.nick_from);
			free(cmd_data.jid_from);
		}
		
		else if (MESSAGE("list online"))
		{
			cmd_data.session = &session;
			cmd_data.nick_from = malloc ( 64 );
			cmd_data.jid_from = malloc ( 64 );
			strcpy(cmd_data.nick_from, nick_from);
			strcpy(cmd_data.jid_from, jid_from);
			list_iterate(2);
			free(cmd_data.nick_from);
			free(cmd_data.jid_from);
		}
		
		else if (MESSAGE("invite online"))
		{
			cmd_data.session = &session;
			cmd_data.nick_from = malloc ( 64 );
			cmd_data.jid_from = malloc ( 64 );
			strcpy(cmd_data.nick_from, nick_from);
			strcpy(cmd_data.jid_from, jid_from);
			list_iterate(3);
			free(cmd_data.nick_from);
			free(cmd_data.jid_from);
		}

		else if (MESSAGE("ready"))
		{
			send_stream_format(session.wfs,
							   "<iq to='masterserver@warface/%s' type='get'>"
							   " <query xmlns='urn:cryonline:k01'>"
							   "  <gameroom_setplayer team_id='0' status='1' class_id='0'/>"
							   " </query>"
							   "</iq>",
							   session.channel);

			xmpp_send_message(session.wfs, session.nickname, session.jid,
							  nick_from, jid_from,
							  "Ready when you are!", NULL);
		}

		else if (MESSAGE("invite"))
		{
			send_stream_format(session.wfs,
							   "<iq to='masterserver@warface/%s' type='get'>"
							   " <query xmlns='urn:cryonline:k01'>"
							   "  <invitation_send nickname='%s' is_follow='0'/>"
							   " </query>"
							   "</iq>",
							   session.channel, nick_from);
		}

		else if (MESSAGE("master"))
		{
			xmpp_promote_room_master(nick_from);

			xmpp_send_message(session.wfs, session.nickname, session.jid,
							  nick_from, jid_from,
							  "Yep, just a sec.", NULL);

		}

		else if (strstr(message, "whois"))
		{
			char *nickname = strchr(message, ' ');

			if (nickname == NULL)
				nickname = nick_from;
			else
				nickname++;

			xmpp_iq_profile_info_get_status(nickname, nick_from, jid_from);

		}

		else if (MESSAGE("help"))
		{
			xmpp_send_message(session.wfs, session.nickname, session.jid,
							  nick_from, jid_from,
							  "leave - Make me leave the room :(", NULL);
			xmpp_send_message(session.wfs, session.nickname, session.jid,
							  nick_from, jid_from,
							  "ready - Set my status to &apos;Ready&apos;.", NULL);
			xmpp_send_message(session.wfs, session.nickname, session.jid,
							  nick_from, jid_from,
							  "invite - Invite you to my room.", NULL);
			xmpp_send_message(session.wfs, session.nickname, session.jid,
							  nick_from, jid_from,
							  "master - Promote you to room master.", NULL);
		}

		else if (
					MESSAGEA("hi") || MESSAGEA("Hi") ||
					MESSAGEA("hey") || MESSAGEA("Hey") ||
					MESSAGEA("hello") || MESSAGEA("Hello") ||
					MESSAGEA("yo") || MESSAGEA("sup") ||
					MESSAGEA("wassup") || MESSAGEA("Wassup")
				)
		{
			char *msg_hi = malloc ( 80 );
			strcpy ( msg_hi, message );
			strcat( msg_hi, " ");
			strcat( msg_hi, nick_from);
			strcat( msg_hi, "!");
			xmpp_send_message(session.wfs, session.nickname, session.jid,
							  nick_from, jid_from,
							  (const char*) msg_hi, NULL);
			xmpp_send_message(session.wfs, session.nickname, session.jid,
							  nick_from, jid_from,
							  "Type &apos;help&apos; for a list of available commands.", NULL);
			free ( msg_hi );
		}

		else
		{
			/* Command not found */
			char *reply = malloc ( 256 );
			strcpy( reply, "I don&apos;t recognize &apos;" );
			strcat ( reply, message );
			strcat ( reply, "&apos; as a valid command." );
			xmpp_send_message(session.wfs, session.nickname, session.jid,
							  nick_from, jid_from,
							  (const char*) reply, NULL);
			xmpp_send_message(session.wfs, session.nickname, session.jid,
							  nick_from, jid_from,
							  "Try &apos;help&apos; to get a list of available commands.", NULL);
			free ( reply );
		}
	}
	else
	{
		char *reply = malloc(512);
		switch ( time(NULL) % 10 )
		{
			case 1:
				strcpy ( reply, "Please don&apos;t curse around me :(" );
				break;
			case 2:
				strcpy ( reply, "Oh just fok off." );
				break;
			case 3:
				strcpy ( reply, "I&apos;m just a bot, but you&apos;re still hurting me :(" );
				break;
			case 4:
				strcpy ( reply, "If you hate me so much then just stop inviting me!" );
				break;
			case 5:
				strcpy ( reply, "What did I ever do to you :(" );
				break;
			case 6:
				strcpy ( reply, "You know what? Screw you. I can find better friends." );
				break;
			case 7:
				strcpy ( reply, "Don&apos;t you dare curse me. I just do as I&apos;m told." );
				break;
			default:
			strcpy ( reply, "Stop it! You&apos;re gonna make me cry! :&apos;(" );
		}
		puts ( reply );
		xmpp_send_message(session.wfs, session.nickname, session.jid,
							  nick_from, jid_from,
							  (const char*) reply, NULL);
		free ( reply );
	}
    free(id);
    free(jid_from);
    free(nick_from);
    free(message);
}

void xmpp_message_r(void)
{
    qh_register("message", xmpp_message_cb);
}
