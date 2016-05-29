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

#include <string.h>
#include <wb_log.h>

#include <wb_session.h>
#include <wb_mission.h>
#include <wb_xmpp_wf.h>

void cmd_open_pvp_cb(const char *room_id, void *args)
{
    enum pvp_mode flags = PVP_AUTOBALANCE | PVP_DEADCHAT;

    if (!session.gameroom.is_safemaster)
        flags |= PVP_ALLOWJOIN | PVP_PRIVATE;

    xmpp_iq_gameroom_update_pvp(
        session.gameroom.sync.mission.mission_key,
        flags,
        session.gameroom.sync.custom_params.max_players,
        session.gameroom.sync.custom_params.inventory_slot,
        NULL, NULL);
}

struct cb_args
{
    char is_pvp;
    char *mission_key;
};

void join_channel_cb(void *args)
{
    struct cb_args *a = (struct cb_args *) args;

    if (a->is_pvp)
        xmpp_iq_gameroom_open(a->mission_key,
                              ROOM_PVP_PUBLIC,
                              cmd_open_pvp_cb, NULL);
    else
        xmpp_iq_gameroom_open(a->mission_key,
                              ROOM_PVE_PRIVATE,
                              NULL, NULL);

    free(a->mission_key);
    free(a);
}

void cmd_open(const char *mission_name)
{
    if (mission_name == NULL)
        mission_name = "trainingmission";

    if (strlen(mission_name) != 36) /* not an uuid */
    {
        struct mission *m = mission_list_get(mission_name);

        if (m != NULL)
        {
            int is_pvp = strstr(m->mode, "pvp") != NULL;
            int were_in_pvp =
                strstr(session.online.channel, "pvp") != NULL;

            struct cb_args *a = calloc(1, sizeof (struct cb_args));
            a->is_pvp = is_pvp;
            a->mission_key = strdup(m->mission_key);

            if (is_pvp && !were_in_pvp)
                xmpp_iq_join_channel("pvp_pro_1", join_channel_cb, a);
            else if (!is_pvp && were_in_pvp)
                xmpp_iq_join_channel("pve_1", join_channel_cb, a);
            else
                join_channel_cb(a);
        }
        else
        {
            eprintf("No such map or mission '%s'\n", mission_name);
        }
    }
    else
    {
        xmpp_iq_gameroom_open(mission_name,
                              ROOM_PVE_PRIVATE,
                              NULL, NULL);
    }
}
