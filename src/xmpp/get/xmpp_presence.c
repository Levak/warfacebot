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

#include <stdlib.h>
#include <string.h>

struct args
{
    int leave;
    char *room_jid;
};

static void xmpp_presence_cb_(const char *msg, void *args)
{
    /* Answer :
       <presence to='xxxxx@warface/GameClient' type='unavailable'>
        <x xmlns='http://jabber.org/protocol/muc#user'>
         <item affiliation='none' role='none'/>
        </x>
       </presence>
     */

    struct args *a = (struct args *) args;

    if (!xmpp_is_error(msg))
    {
        /* TODO: multiple rooms ? */

        if (a->leave)
        {
            free(session.room_jid);
            session.room_jid = NULL;
        }
        else
        {
            session.room_jid = strdup(a->room_jid);
        }
    }

    free(a->room_jid);
    free(a);
}

void xmpp_presence(const char *room_jid, int leave)
{
    struct args *a = calloc(1, sizeof (struct args));

    a->leave = leave;
    a->room_jid = strdup(room_jid);

    t_uid id;

    idh_generate_unique_id(&id);
    idh_register(&id, 0, xmpp_presence_cb_, a);

    if (leave)
    {
        /* Leave the XMPP room */
        send_stream_format(session.wfs,
                           "<presence id='%s' to='%s' type='unavailable'/>",
                           &id, room_jid);
    }
    else
    {
        /* Join the XMPP room */
        send_stream_format(session.wfs,
                           "<presence id='%s' to='%s/%s'/>",
                           &id, room_jid, session.nickname);
    }
}

