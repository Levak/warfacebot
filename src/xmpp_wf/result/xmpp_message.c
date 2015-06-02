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
#include <wb_xml.h>
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>
#include <wb_session.h>
#include <wb_mission.h>
#include <wb_list.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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

    /* Deserialize message */

    xml_deserialize_inplace(&message);

    /* Feedback the user what was sent */

    xmpp_send_message(session.wfs, nick_from, session.jid,
                      session.nickname, jid_from,
                      message, msg_id);

    /* Determine the correct command */

    if (strstr(message, "leave"))
    {
        xmpp_iq_gameroom_leave();

        xmpp_send_message(session.wfs, session.nickname, session.jid,
                          nick_from, jid_from,
                          "but whyy :(", NULL);
    }

    else if (strstr(message, "ready"))
    {
        xmpp_iq_gameroom_setplayer(0, 1, 0, NULL, NULL);
        xmpp_send_message(session.wfs, session.nickname, session.jid,
                          nick_from, jid_from,
                          "go", NULL);
    }

    else if (strstr(message, "invite"))
    {
        xmpp_iq_invitation_send(nick_from, 0, NULL, NULL);
    }

    else if (strstr(message, "master"))
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

    else
    {
        int r = rand() % 4;
        const char *answer =
            r == 0 ? "I'm sorry Dave. I'm afraid I can't do that." :
            r == 1 ? "It can only be attributable to human error." :
            r == 2 ? "Just what do you think you're doing, Dave ?" :
            "Dave, stop. Stop, will you ?";

        /* Command not found */
        xmpp_send_message(session.wfs, session.nickname, session.jid,
                          nick_from, jid_from,
                          answer, NULL);
    }

    free(jid_from);
    free(nick_from);
    free(message);
}

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
