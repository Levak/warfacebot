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
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>
#include <wb_session.h>

#include <stdlib.h>
#include <wb_log.h>

struct cb_args
{
    f_invitation_result_cb cb;
    void *args;
};

static void invitation_result_cb(const char *msg_id,
                                 const char *msg,
                                 void *args)
{
    struct cb_args *a = (struct cb_args *) args;

    /* Answer:
       <iq from='masterserver@warface/xxxxxxx' id='uid0002d87c' type='get'>
        <query xmlns='urn:cryonline:k01'>
         <invitation_result result='17' user='xxxxxxx' is_follow='1' user_id='xxxxxx'/>
        </query>
       </iq>
     */

    const char *reason = "Unknown reason";
    int result = INVIT_ERROR;
    char *channel = NULL;

    if (msg != NULL)
    {
        result = get_info_int(msg, "result='", "'", NULL);

        switch (result)
        {
            case INVIT_USER_NOT_IN_ROOM:
                reason = "User not in a room";
                break;
            case INVIT_FULL_ROOM:
                reason = "Room is full";
                break;
            case INVIT_KICKED:
                reason = "Kicked from room";
                break;
            case INVIT_PRIVATE_ROOM:
                reason = "Room is private";
                break;
            case INVIT_NOT_IN_CLAN:
            case INVIT_NOT_IN_CW:
                reason = "Room is a clanwar";
                break;
            default:
                break;
        }

        channel = get_info(msg, "from='masterserver@warface/", "'", NULL);
    }

    xprintf("Failed to follow (%s).\n", reason);

    if (a->cb)
        a->cb(channel, result, a->args);

    free(channel);
    free(a);
}

void xmpp_iq_follow_send(const char *online_id, f_invitation_result_cb cb, void *args)
{
    struct cb_args *a = calloc(1, sizeof (struct cb_args));
    a->cb = cb;
    a->args = args;

    qh_remove("invitation_result");
    qh_register("invitation_result", 0, invitation_result_cb, a);

    send_stream_format(session.wfs,
                       "<iq to='%s' type='get'>"
                       " <query xmlns='urn:cryonline:k01'>"
                       "  <follow_send nickname='%s' profile_id='%s'/>"
                       " </query>"
                       "</iq>",
                       online_id,
                       session.profile.nickname,
                       session.profile.id);
}
