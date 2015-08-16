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
#include <wb_session.h>
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct cb_args
{
    char *channel;
    char *room_id;
};

static void xmpp_iq_gameroom_join_cb(const char *msg, void *args)
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

    if (!xmpp_is_error(msg))
    {

        session.ingameroom = 1;

        /* Leave previous room if any */
        if (session.gameroom_jid != NULL)
        {
            xmpp_presence(session.gameroom_jid, 1);
            free(session.gameroom_jid);
            session.gameroom_jid = NULL;
        }

        /* Join XMPP room */
        char *room_jid;

        FORMAT(room_jid, "room.%s.%s@conference.warface",
               a->channel, a->room_id);

        xmpp_presence(room_jid, 0);
        session.gameroom_jid = room_jid;

        /* Change public status */
        xmpp_iq_player_status(STATUS_ONLINE | STATUS_ROOM);
    }

    free(a->room_id);
    free(a->channel);
    free(a);
}

static void xmpp_iq_gameroom_join_(void *args)
{
    struct cb_args *a = (struct cb_args *) args;

    t_uid id;

    idh_generate_unique_id(&id);
    idh_register(&id, 0, xmpp_iq_gameroom_join_cb, args);

    /* Open the game room */
    send_stream_format(session.wfs,
                       "<iq id='%s' to='masterserver@warface/%s' type='get'>"
                       " <query xmlns='urn:cryonline:k01'>"
                       "  <gameroom_join room_id='%s' team_id='0'"
                       "     status='1' class_id='1' join_reason='0'/>"
                       " </query>"
                       "</iq>",
                       &id, session.channel, a->room_id);
}

void xmpp_iq_gameroom_join(const char *channel, const char *room_id)
{
    struct cb_args *a = calloc(1, sizeof (struct cb_args));
    a->channel = strdup(channel);
    a->room_id = strdup(room_id);

    /* Change channel if room is not on the same server */
    if (strcmp(session.channel, channel))
        xmpp_iq_join_channel(channel, xmpp_iq_gameroom_join_, a);
    else
        xmpp_iq_gameroom_join_(a);
}

