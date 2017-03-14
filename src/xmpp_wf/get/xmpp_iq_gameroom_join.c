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
#include <wb_session.h>
#include <wb_xmpp.h>
#include <wb_quickplay.h>
#include <wb_xmpp_wf.h>
#include <wb_status.h>

#include <stdlib.h>
#include <string.h>
#include <time.h>

struct cb_args
{
    char *room_id;
    char *group_id;
    enum join_reason reason;
};

static void xmpp_iq_gameroom_join_cb(const char *msg,
                                     enum xmpp_msg_type type,
                                     void *args)
{
    /* Answer :
       <iq to='masterserver@warface/pve_2' type='get'>
        <query xmlns='urn:cryonline:k01'>
         <data query_name='gameroom_join' compressedData='...'
               originalSize='42'/>
        </query>
       </iq>
     */

    struct cb_args *a = (struct cb_args *) args;

    if (type ^ XMPP_TYPE_ERROR)
    {
        char *data = wf_get_query_content(msg);

        if (data == NULL)
            return;

        /* Leave previous room if any */
        if (session.gameroom.jid != NULL)
        {
            xmpp_presence(session.gameroom.jid, XMPP_PRESENCE_LEAVE, NULL, NULL);
            free(session.gameroom.group_id);
            session.gameroom.group_id = NULL;
            free(session.gameroom.jid);
            session.gameroom.jid = NULL;

            gameroom_sync_free();
        }

        /* Clean quickplay settings */
        quickplay_free();

        /* Join XMPP room */
        char *room_jid;

        FORMAT(room_jid, "room.%s.%s@%s",
               session.online.channel,
               a->room_id,
               session.online.jid.muc);

        xmpp_presence(room_jid, XMPP_PRESENCE_JOIN, NULL, NULL);
        session.gameroom.jid = room_jid;

        if (a->group_id != NULL)
            session.gameroom.group_id = strdup(a->group_id);

        /* Reset auto-ready */
        session.gameroom.desired_status = GAMEROOM_READY;

        gameroom_sync_init();
        gameroom_sync(data);

        /* Change public status */
        status_set(STATUS_ONLINE | STATUS_ROOM);

        session.gameroom.leave_timeout = time(NULL);

        free(data);
    }

    free(a->room_id);
    free(a->group_id);
    free(a);
}

static void xmpp_iq_gameroom_join_(void *args)
{
    struct cb_args *a = (struct cb_args *) args;

    /* Open the game room */
    xmpp_send_iq_get(
        JID_MS(session.online.channel),
        xmpp_iq_gameroom_join_cb, args,
        "<query xmlns='urn:cryonline:k01'>"
        " <gameroom_join room_id='%s' team_id='0'"
        "    group_id='%s'"
        "    status='%d' class_id='%d' join_reason='%d'"
        "    wait_time_to_join='0'/>"
        "</query>",
        a->room_id,
        a->group_id ? a->group_id : "",
        GAMEROOM_UNREADY,
        session.profile.curr_class,
        a->reason);
}

void xmpp_iq_gameroom_join(const char *channel,
                           const char *room_id,
                           const char *group_id,
                           enum join_reason reason)
{
    if (room_id == NULL || channel == NULL)
        return;

    struct cb_args *a = calloc(1, sizeof (struct cb_args));

    a->room_id = strdup(room_id);
    a->group_id = group_id ? strdup(group_id) : NULL;
    a->reason = reason;

    /* Change channel if room is not on the same server */
    if (strcmp(session.online.channel, channel))
        xmpp_iq_join_channel(channel, xmpp_iq_gameroom_join_, a);
    else
        xmpp_iq_gameroom_join_(a);
}

