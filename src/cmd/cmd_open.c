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

#include <wb_log.h>
#include <wb_cvar.h>
#include <wb_session.h>
#include <wb_mission.h>
#include <wb_xmpp_wf.h>
#include <wb_masterserver.h>
#include <wb_cmd.h>
#include <wb_lang.h>

#include <string.h>

void cmd_open_pvp_cb(const char *room_id, void *args)
{
    enum pvp_mode flags = PVP_AUTOBALANCE | PVP_DEADCHAT;

    if (!cvar.wb_safemaster)
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
        mission_name = "training";

    if (strlen(mission_name) != 36) /* not an uuid */
    {
        struct mission *m = mission_list_get(mission_name);

        if (m != NULL)
        {
            int is_pve = strstr(m->mode, "pvp") == NULL;
            int were_in_pve =
                strstr(session.online.channel_type, "pve") != NULL;

            const char *ms_type = is_pve ? "pve" : "pvp_pro";

            const struct masterserver *ms =
                masterserver_list_get_by_type(ms_type);

            if (ms != NULL)
            {
                struct cb_args *a = calloc(1, sizeof (struct cb_args));
                a->is_pvp = !is_pve;
                a->mission_key = strdup(m->mission_key);

                if (is_pve == were_in_pve)
                {
                    join_channel_cb(a);
                }
                else
                {
                    xmpp_iq_join_channel(ms->resource,
                                         join_channel_cb,
                                         a);
                }
            }
            else
            {
                char *s = LANG_FMT(error_no_channel_type,
                                   ms_type);
                eprintf("%s", s);
                free(s);
            }
        }
        else
        {
            eprintf("%s: '%s'",
                    LANG(error_no_map),
                    mission_name);
        }
    }
    else
    {
        xmpp_iq_gameroom_open(mission_name,
                              ROOM_PVE_PRIVATE,
                              NULL, NULL);
    }
}

void cmd_open_wrapper(const char *mission_name)
{
    cmd_open(mission_name);
}

int cmd_open_completions(struct list *l)
{
    complete_missions_pve(l);
    complete_missions_pvp(l);

    return 1;
}
