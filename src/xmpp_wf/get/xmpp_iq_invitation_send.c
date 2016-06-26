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
#include <wb_session.h>
#include <wb_xml.h>
#include <wb_xmpp_wf.h>

#include <stdlib.h>
#include <wb_log.h>

struct cb_args
{
    f_invitation_result_cb cb;
    void *args;
};

static void invitation_failure(const char *nickname,
                               int code,
                               int custom_code)
{
    const char *reason = NULL;

    switch (code)
    {
        case 8:
            switch (custom_code)
            {
                case INVIT_DUPLICATE:
                    reason = "Already in the room";
                    break;
                case INVIT_USER_OFFLINE:
                    reason = "No such connected user";
                    break;
                case INVIT_USER_NOT_IN_ROOM:
                    reason = "We are not in a room";
                    break;
                case INVIT_FULL_ROOM:
                    reason = "Room is full";
                    break;
                case INVIT_KICKED:
                    reason = "Kicked from room";
                    break;
                case INVIT_NOT_IN_CLAN:
                case INVIT_NOT_IN_CW:
                    reason = "Room is a clanwar";
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }

    if (reason != NULL)
        eprintf("Failed to invite %s (%s)\n",
                nickname, reason);
    else
        eprintf("Failed to invite %s (%i:%i)\n",
                nickname, code, custom_code);
}

static void invitation_result_cb(const char *msg_id,
                                 const char *msg,
                                 void *args)
{
    struct cb_args *a = (struct cb_args *) args;

    /* Answer:
       <iq from='masterserver@warface/xxxxxxx' id='uid0002d87c' type='get'>
        <query xmlns='urn:cryonline:k01'>
         <invitation_result result='17' user='xxxxxxx' is_follow='0' user_id='xxxxxx'/>
        </query>
       </iq>
     */

    int result = INVIT_ERROR;
    char *nickname = NULL;

    if (msg != NULL)
    {
        result = get_info_int(msg, "result='", "'", NULL);
        nickname = get_info(msg, "user='", "'", NULL);
    }

    if (result != INVIT_ACCEPTED)
        invitation_failure(nickname, 8, result);

    if (a->cb)
        a->cb(NULL, result, a->args);

    free(a);
    free(nickname);
}

static void xmpp_iq_invitation_send_cb(const char *msg,
                                       enum xmpp_msg_type type,
                                       void *args)
{
    /* Answer :
       <iq to='masterserver@warface/pve_2' type='get'>
        <query xmlns='urn:cryonline:k01'>
         <invitation_send/>
        </query>
       </iq>
     */

    if (type & XMPP_TYPE_ERROR && msg != NULL)
    {
        char *nickname = get_info(msg, "nickname='", "'", NULL);
        int code = get_info_int(msg, "code='", "'", NULL);
        int custom_code = get_info_int(msg, "custom_code='", "'", NULL);

        invitation_failure(nickname, code, custom_code);

        free(nickname);
    }
}

void xmpp_iq_invitation_send(const char *nickname, int is_follow,
                             f_invitation_result_cb cb, void *args)
{
    if (session.gameroom.jid != NULL)
    {
        char *nick = xml_serialize(nickname);

        struct cb_args *a = calloc(sizeof (struct cb_args), 1);

        a->cb = cb;
        a->args = args;

        /* TODO: Handle multiple invitation errors at once is not supported */
        qh_remove("invitation_result");
        qh_register("invitation_result", 0, invitation_result_cb, a);

        if (session.gameroom.group_id == NULL)
            session.gameroom.group_id = new_random_uuid();

        xmpp_send_iq_get(
            JID_MS(session.online.channel),
            xmpp_iq_invitation_send_cb, NULL,
            " <query xmlns='urn:cryonline:k01'>"
            "  <invitation_send nickname='%s' is_follow='%d'"
            "                   group_id='%s'/>"
            " </query>",
            nick,
            is_follow,
            session.gameroom.group_id);

        free(nick);
    }
}

