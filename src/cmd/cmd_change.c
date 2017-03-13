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

#include <wb_session.h>
#include <wb_mission.h>
#include <wb_xmpp_wf.h>
#include <wb_pvp_maps.h>
#include <wb_cmd.h>

#include <string.h>

void cmd_change(const char *mission_name)
{
    struct mission *m = NULL;

    if (strstr(session.online.channel_type, "pvp"))
    {
        if (mission_name == NULL)
            mission_name = "tdm_airbase";

        m = mission_list_get(mission_name);
        enum pvp_mode flags = 0;

        if (session.gameroom.sync.core.private)
            flags |= PVP_PRIVATE;
        if (session.gameroom.sync.custom_params.auto_team_balance)
            flags |= PVP_AUTOBALANCE;
        if (session.gameroom.sync.custom_params.dead_can_chat)
            flags |= PVP_DEADCHAT;
        if (session.gameroom.sync.custom_params.join_in_the_process)
            flags |= PVP_ALLOWJOIN;

        if (m != NULL)
        {
            xmpp_iq_gameroom_update_pvp(
                m->mission_key,
                flags,
                session.gameroom.sync.custom_params.max_players,
                session.gameroom.sync.custom_params.inventory_slot,
                NULL, NULL);
        }
        else
        {
            xmpp_iq_gameroom_update_pvp(
                mission_name,
                flags,
                session.gameroom.sync.custom_params.max_players,
                session.gameroom.sync.custom_params.inventory_slot,
                NULL, NULL);
        }
    }
    else
    {
        if (mission_name == NULL)
            mission_name = "trainingmission";

        m = mission_list_get(mission_name);

        if (m != NULL)
        {
            xmpp_iq_gameroom_setinfo(m->mission_key, NULL, NULL);
        }
        else
        {
            xmpp_iq_gameroom_setinfo(mission_name, NULL, NULL);
        }
    }
}

void cmd_change_wrapper(const char *mission_name)
{
    cmd_change(mission_name);
}

int cmd_change_completions(struct list *l)
{
    if (session.online.channel == NULL)
        return 1;

    if (strstr(session.online.channel_type, "pve"))
        complete_missions_pve(l);
    else
        complete_missions_pvp(l);

    return 1;
}

