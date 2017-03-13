/**
 * WarfaceBot, a blind XMPP client for Warface (FPS)
 * Copyright (C) 2015-2017 Levak Borok <levak92@gmail.com>
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
#include <wb_xml.h>
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>
#include <wb_session.h>
#include <wb_mission.h>
#include <wb_list.h>
#include <wb_cmd.h>
#include <wb_dbus.h>
#include <wb_cvar.h>
#include <wb_log.h>

#include <stdlib.h>
#include <string.h>

static void handle_room_message_(const char *msg_id, const char *msg)
{
    /* Answer #1:
       <message from='room.pve_12.xxxx@conference.warface/xxxx'
                to='xxxx@warface/GameClient' type='groupchat'>
        <body>l</body>
       </message>
    */

    if (msg == NULL)
        return;

    char *message = get_info(msg, "<body>", "</body>", NULL);
    char *from = get_info(msg, "from=", " ", NULL);
    char *nick_from = get_info(from, "/", "'", NULL);
    char *room_jid = get_info(from, "'", "/", NULL);

    if (message == NULL || nick_from == NULL || room_jid == NULL)
    {
        free(message);
        free(nick_from);
        free(room_jid);
        free(from);

        return;
    }

    char *saveptr;
    char *simple_rjid = strdup(strtok_r(room_jid, "@", &saveptr));

    /* Deserialize message */

    xml_deserialize_inplace(&message);

#ifdef DBUS_API
    dbus_api_emit_room_message(room_jid, nick_from, message);
#endif /* DBUS_API */

    if (strstr(room_jid, "clan"))
        xprintf("%s    \t\033[33;1m%s\033[0m: %s", simple_rjid, nick_from, message);
    else if (strstr(room_jid, "global"))
        xprintf("%s    \t\033[31;1m%s\033[0m: %s", simple_rjid, nick_from, message);
    else
        xprintf("%s    \t\033[35;1m%s\033[0m: %s", simple_rjid, nick_from, message);

    if (strstr(room_jid, "global"))
    {
    }
    else
    {
        if (strcmp(nick_from, session.profile.nickname) != 0)
        {
            if (cvar.wb_safemaster
                && (strcasestr(message, "start") != NULL
                    || strcasecmp(message, "go") == 0))
            {
                cmd_start();
            }
        }
    }

    /* TODO */

    free(simple_rjid);
    free(room_jid);
    free(message);
    free(nick_from);
    free(from);
}

static const char *random_answer(
    const char *a1,
    const char *a2,
    const char *a3,
    const char *a4)
{
    srand(time(NULL));

    int r = rand() % 4;
    const char *answer =
        r == 0 ? a1 :
        r == 1 ? a2 :
        r == 2 ? a3 :
        a4;

    return answer;
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

    char *data = wf_get_query_content(msg);

    if (data == NULL)
        return;

    char *message_node = get_info(data, "<message", "/>", NULL);

    if (message_node == NULL)
    {
        free(data);
        return;
    }

    char *message = get_info(message_node, "message='", "'", NULL);
    char *nick_from = get_info(message_node, "from='", "'", NULL);
    char *jid_from = get_info(msg, "from='", "'", NULL);

    /* Deserialize message */

    xml_deserialize_inplace(&message);

    xprintf("\033[32;1m%s\033[0m: %s", nick_from, message);

#ifdef DBUS_API
    dbus_api_emit_buddy_message(nick_from, message);
#endif /* DBUS_API */

    /* Feedback the user what was sent */

    xmpp_ack_message(nick_from, jid_from, message, msg_id);

    if (!cvar.wb_enable_whisper_commands)
    {
        free(jid_from);
        free(nick_from);
        free(message);
        free(message_node);
        free(data);
        return;
    }

    /* Determine the correct command */

    if (strstr(message, "leave"))
    {
        cmd_leave();

        const char *answer = random_answer(
            LANG(whisper_leave_1),
            LANG(whisper_leave_2),
            LANG(whisper_leave_3),
            LANG(whisper_leave_4));

        xmpp_send_message(nick_from, jid_from, answer);
    }

    else if (strstr(message, "unready"))
    {
        cmd_unready();

        const char *answer = random_answer(
            LANG(whisper_unready_1),
            LANG(whisper_unready_2),
            LANG(whisper_unready_3),
            LANG(whisper_unready_4));

        xmpp_send_message(nick_from, jid_from, answer);
    }

    else if (strstr(message, "ready") || strstr(message, "take"))
    {
        cmd_ready(strstr(message, " "));

        const char *answer = random_answer(
            LANG(whisper_ready_1),
            LANG(whisper_ready_2),
            LANG(whisper_ready_3),
            LANG(whisper_ready_4));

        xmpp_send_message(nick_from, jid_from, answer);
    }

    else if (cvar.wb_enable_invite && strstr(message, "invite"))
    {
        cmd_invite(nick_from);
    }

    else if (strstr(message, "follow"))
    {
        char *nickname = strchr(message, ' ');

        if (nickname == NULL)
            nickname = nick_from;
        else
            nickname++;

        cmd_follow(nickname);
    }

    else if (strstr(message, "master"))
    {
        cmd_master(nick_from);

        const char *answer = random_answer(
            LANG(whisper_master_1),
            LANG(whisper_master_2),
            LANG(whisper_master_3),
            LANG(whisper_master_4));

        xmpp_send_message(nick_from, jid_from, answer);

    }

    else if (strstr(message, "last"))
    {
        char *nickname = strchr(message, ' ');

        if (nickname == NULL)
            nickname = nick_from;
        else
            nickname++;

        cmd_last(nickname,
                 cmd_last_whisper_cb,
                 cmd_whisper_args(nick_from, jid_from));
    }

    else if (strstr(message, "whois"))
    {
        char *nickname = strchr(message, ' ');

        if (nickname == NULL)
            nickname = nick_from;
        else
            nickname++;

        cmd_whois(nickname,
                  cmd_whois_whisper_cb,
                  cmd_whisper_args(nick_from, jid_from));
    }

    else if (strstr(message, "missions"))
    {
        cmd_missions(cmd_missions_whisper_cb,
                     cmd_whisper_args(nick_from, jid_from));
    }

    else if (strstr(message, "say"))
    {
        cmd_say(strchr(message, ' '));
    }

    else if (strstr(message, "stay"))
    {
        if (session.gameroom.jid != NULL)
        {
            cmd_stay(60 * 60);

            const char *answer = random_answer(
                LANG(whisper_stay_1),
                LANG(whisper_stay_2),
                LANG(whisper_stay_3),
                LANG(whisper_stay_4));

            xmpp_send_message(nick_from, jid_from, answer);
        }
        else
        {
            const char *answer = random_answer(
                LANG(whisper_stay_ko_1),
                LANG(whisper_stay_ko_2),
                LANG(whisper_stay_ko_3),
                LANG(whisper_stay_ko_4));

            xmpp_send_message(nick_from, jid_from, answer);
        }
    }

    else if (strstr(message, "start"))
    {
        cmd_start();
    }

    else if (strstr(message, "switch"))
    {
        cmd_switch();
    }

    else
    {
        const char *answer = random_answer(
            LANG(whisper_unknown_1),
            LANG(whisper_unknown_2),
            LANG(whisper_unknown_3),
            LANG(whisper_unknown_4));

        /* Command not found */
        xmpp_send_message(nick_from, jid_from, answer);
    }

    free(jid_from);
    free(nick_from);
    free(message);
    free(message_node);
    free(data);
}

static void xmpp_message_cb(const char *msg_id, const char *msg, void *args)
{
    char *type = get_info(msg, "type='", "'", NULL);

    if (strcmp(type, "groupchat") == 0)
        handle_room_message_(msg_id, msg);

    else if (strcmp(type, "get") == 0)
        handle_private_message_(msg_id, msg);

    free(type);
}

void xmpp_message_r(void)
{
    qh_register("message", 1, xmpp_message_cb, NULL);
}
