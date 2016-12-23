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
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>
#include <wb_log.h>
#include <wb_status.h>
#include <wb_mission.h>

#include <stdlib.h>

struct cb_args
{
    char *mission_key;
    char *mission_name;
    enum room_type type;
    f_gameroom_open_cb fun;
    void *args;
    int tries;
};

/* Forward declaration */
static void _xmpp_iq_gameroom_open(const char *mission_key,
                                   enum room_type type,
                                   int tries,
                                   f_gameroom_open_cb fun,
                                   void *args);

static void _open_updated_list(void *args)
{
    struct cb_args *a = (struct cb_args *) args;
    struct mission *m = mission_list_get(a->mission_name);

    if (m != NULL)
    {
        _xmpp_iq_gameroom_open(m->mission_key,
                               a->type,
                               a->tries,
                               a->fun,
                               a->args);
    }
    else
    {
        eprintf("Failed to open room (Expired missions)\n");
    }

    free(a->mission_key);
    a->mission_key = NULL;
    free(a->mission_name);
    a->mission_name = NULL;
    free(a);
}

static void xmpp_iq_gameroom_open_cb(const char *msg,
                                     enum xmpp_msg_type type,
                                     void *args)
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

    if (type & XMPP_TYPE_ERROR)
    {
        int code = get_info_int(msg, "code='", "'", NULL);
        int custom_code = get_info_int(msg, "custom_code='", "'", NULL);
        const char *reason = NULL;

        switch (code)
        {
            case 1006:
                reason = "QoS limit reached";
                break;
            case 8:
                switch (custom_code)
                {
                    case 0: /* Expired mission, update and try again */
                        if (++a->tries < 2)
                        {
                            struct mission *m =
                                mission_list_get_by_key(a->mission_key);

                            if (m != NULL)
                            {
                                a->mission_name = strdup(m->name);
                                mission_list_update(
                                    _open_updated_list,
                                    args);

                                return;
                            }
                        }

                        reason = "Expired missions";
                        break;
                    case 1:
                        reason = "Invalid or expired mission";
                        break;
                    case 12:
                        reason = "Rank restricted";
                        break;
                    case 21:
                        reason = "Invalid room name";
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }

        if (reason != NULL)
            eprintf("Failed to open room (%s)\n", reason);
        else
            eprintf("Failed to open room (%i:%i)\n", code, custom_code);
    }
    else
    {
        char *data = wf_get_query_content(msg);

        if (data == NULL)
        {
            free(a);
            return;
        }

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

        char *room = get_info(data, "room_id='", "'", "Room ID");

        if (room != NULL)
        {
            /* Join XMPP room */
            char *room_jid;

            FORMAT(room_jid, "room.%s.%s@conference.warface",
                   session.online.channel, room);
            xmpp_presence(room_jid, XMPP_PRESENCE_JOIN, NULL, NULL);
            session.gameroom.jid = room_jid;

            /* Reset auto-ready */
            session.gameroom.desired_status = GAMEROOM_READY;

            gameroom_sync_init();
            gameroom_sync(data);

            status_set(STATUS_ONLINE | STATUS_ROOM);
        }

        if (a->fun != NULL)
            a->fun(room, a->args);

        free(room);

        free(data);
    }

    free(a->mission_name);
    a->mission_name = NULL;
    free(a->mission_key);
    a->mission_key = NULL;
    free(a);
}

static void _xmpp_iq_gameroom_open(const char *mission_key,
                                   enum room_type type,
                                   int tries,
                                   f_gameroom_open_cb fun,
                                   void *args)
{
    struct cb_args *a = calloc(1, sizeof (struct cb_args));

    a->fun = fun;
    a->args = args;
    a->tries = tries;

    if (mission_key != NULL)
        a->mission_key = strdup(mission_key);

    a->type = type;

    /* Open the game room */
    xmpp_send_iq_get(
        JID_MS(session.online.channel),
        xmpp_iq_gameroom_open_cb, a,
        "<query xmlns='urn:cryonline:k01'>"
        " <gameroom_open"
        "     room_name='Room' team_id='0' status='%d'"
        "     class_id='%d' room_type='%d' private='1'"
        "     mission='%s' inventory_slot='0'>"
        " </gameroom_open>"
        "</query>",
        GAMEROOM_UNREADY,
        session.profile.curr_class,
        type,
        mission_key);
}

void xmpp_iq_gameroom_open(const char *mission_key,
                           enum room_type type,
                           f_gameroom_open_cb fun,
                           void *args)
{
    if (mission_key == NULL)
        return;

    _xmpp_iq_gameroom_open(mission_key,
                           type,
                           0,
                           fun,
                           args);
}
