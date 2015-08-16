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

struct cb_args
{
    f_gameroom_open_cb fun;
    void *args;
};

static void xmpp_iq_gameroom_open_cb(const char *msg, void *args)
{
    /* Answer :
       <iq to='masterserver@warface/pve_2' type='get'>
        <query xmlns='urn:cryonline:k01'>
         <data query_name='gameroom_open' compressedData='...'
               originalSize='42'/>
        </query>
       </iq>
     */

    struct cb_args *a = (struct cb_args *) args;

    if (xmpp_is_error(msg))
    {
        free(a);
        return;
    }

    session.ingameroom = 1;

    /* Leave previous room if any */
    if (session.gameroom_jid != NULL)
    {
        xmpp_presence(session.gameroom_jid, 1);
        free(session.gameroom_jid);
        session.gameroom_jid = NULL;
    }

    xmpp_iq_player_status(STATUS_ONLINE | STATUS_ROOM);

    char *data = wf_get_query_content(msg);
    char *room = get_info(data, "room_id='", "'", "Room ID");

    free(data);

    /* Join XMPP room */
    char *room_jid;

    FORMAT(room_jid, "room.%s.%s@conference.warface", session.channel, room);
    xmpp_presence(room_jid, 0);
    session.gameroom_jid = room_jid;

    if (a->fun != NULL)
        a->fun(room, a->args);

    free(room);

    free(a);
}

void xmpp_iq_gameroom_open(const char *mission_key,
                           f_gameroom_open_cb fun, void *args)
{
    struct cb_args *a = calloc(1, sizeof (struct cb_args));
    a->fun = fun;
    a->args = args;

    t_uid id;

    idh_generate_unique_id(&id);
    idh_register(&id, 0, xmpp_iq_gameroom_open_cb, a);

    /* Open the game room */
    send_stream_format(session.wfs,
                       "<iq id='%s' to='masterserver@warface/%s' type='get'>"
                       " <query xmlns='urn:cryonline:k01'>"
                       "  <gameroom_open"
                       "      room_name='Room' team_id='1' status='1'"
                       "      class_id='1' room_type='1' private='1'"
                       "      mission='%s' inventory_slot='0'>"
                       "  </gameroom_open>"
                       " </query>"
                       "</iq>",
                       &id, session.channel, mission_key);
}

