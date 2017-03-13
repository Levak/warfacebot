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

#include <wb_room.h>
#include <wb_tools.h>
#include <wb_session.h>
#include <wb_xmpp.h>
#include <wb_lang.h>
#include <wb_log.h>

#include <stdlib.h>
#include <string.h>

struct args
{
    enum xmpp_presence_type type;
    char *room_jid;
    f_presence_cb cb;
    void *args;
};

static void xmpp_presence_cb_(const char *msg,
                              enum xmpp_msg_type type,
                              void *args)
{
    /* Answer :
       <presence to='xxxxx@warface/GameClient' type='unavailable'>
        <x xmlns='http://jabber.org/protocol/muc#user'>
         <item affiliation='none' role='none'/>
        </x>
       </presence>
     */

    struct args *a = (struct args *) args;
    int leave = a->type == XMPP_PRESENCE_LEAVE;

    if (type ^ XMPP_TYPE_ERROR)
    {
        if (leave)
        {
            xprintf("%s %s", LANG(room_leave), a->room_jid);
            room_list_remove(a->room_jid);
        }
        else
        {
            xprintf("%s %s", LANG(room_join), a->room_jid);
            room_list_add(a->room_jid);
        }

        if (a->cb != NULL)
            a->cb(a->room_jid, type, a->args);
    }
    else
    {
        if (leave)
            xprintf("%s %s", LANG(room_leave_fail), a->room_jid);
        else
            xprintf("%s %s", LANG(room_join_fail), a->room_jid);
    }

    free(a->room_jid);
    free(a);
}

void xmpp_presence(const char *room_jid,
                   enum xmpp_presence_type type,
                   f_presence_cb cb, void *args)
{
    if (room_jid == NULL)
        return;

    int leave = type == XMPP_PRESENCE_LEAVE;
    int r = list_get(session.xmpp.rooms, room_jid) != NULL;

    if ((leave && !r) || (!leave && r))
    {
        if (leave)
            xprintf("%s: %s",
                    LANG(error_leave_room),
                    room_jid);
        else
            xprintf("%s: %s",
                    LANG(error_join_room),
                    room_jid);
        return;
    }

    struct args *a = calloc(1, sizeof (struct args));

    a->type = type;
    a->room_jid = strdup(room_jid);
    a->cb = cb;
    a->args = args;

    t_uid id;

    idh_generate_unique_id(&id);
    idh_register(&id, 0, xmpp_presence_cb_, a);

    if (leave)
    {
        /* Leave the XMPP room */
        xmpp_send(
            "<presence id='%s' to='%s' type='unavailable'/>",
            &id, room_jid);
    }
    else
    {
        /* Join the XMPP room */
        xmpp_send(
            "<presence id='%s' to='%s/%s'/>",
            &id, room_jid, session.profile.nickname);
    }
}

