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
#include <wb_friend.h>
#include <wb_clanmate.h>
#include <wb_mission.h>
#include <wb_tools.h>
#include <wb_xmpp_wf.h>
#include <wb_dbus.h>

#include <time.h>

static void status_update_location(void)
{
    free(session.online.place_token);
    session.online.place_token = NULL;
    free(session.online.place_info_token);
    session.online.place_info_token = NULL;
    free(session.online.mode_info_token);
    session.online.mode_info_token = NULL;
    free(session.online.mission_info_token);
    session.online.mission_info_token = NULL;

    if (session.online.status & STATUS_LOBBY)
    {
        session.online.place_token =
            strdup("@ui_playerinfo_inlobby");
    }
    else if (session.gameroom.jid != NULL)
    {
        const char *mode = session.gameroom.sync.mission.mode;

        if (mode != NULL)
        {
            if (strcmp(mode, "pve") == 0)
            {
                session.online.place_token =
                    strdup("@ui_playerinfo_pveroom");

                if (session.gameroom.sync.mission.mission_key != NULL)
                {
                    const struct mission *m =
                        mission_list_get_by_key(
                            session.gameroom.sync.mission.mission_key);

                    if (m != NULL && m->type != NULL)
                    {
                        FORMAT(session.online.place_info_token,
                               "@%s",
                               m->type);
                    }
                }
            }
            else
            {
                session.online.place_token =
                    strdup("@ui_playerinfo_pvproom");
                session.online.place_info_token =
                    strdup("@ui_playerinfo_location");

                if (session.gameroom.sync.mission.mode_name != NULL)
                    session.online.mode_info_token =
                        strdup(session.gameroom.sync.mission.mode_name);
                if (session.gameroom.sync.mission.name != NULL)
                    session.online.mission_info_token =
                        strdup(session.gameroom.sync.mission.name);
            }
        }
    }
}

void status_set(enum status status)
{
    enum status old_status = session.online.status;
    time_t now = time(NULL);

    /* Check if status is different */
    if ((old_status ^ STATUS_AFK)
         != (status ^ STATUS_AFK))
    {
        session.online.last_status_change = now;
    }

    xmpp_iq_player_status(status);

    session.online.status = status;
    session.online.last_status_update = now;

    /* Update cached infos */
    status_update_location();

    /* Broadcast to every buddy */

    list_foreach(session.profile.friends,
                 (f_list_callback) xmpp_iq_peer_status_update_friend,
                 NULL);

    list_foreach(session.profile.clanmates,
                 (f_list_callback) xmpp_iq_peer_clan_member_update_clanmate,
                 NULL);

#ifdef DBUS_API
    /* Broadcast to DBus */
    dbus_api_emit_status_update(session.profile.nickname,
                                session.online.status,
                                session.profile.experience,
                                session.profile.clan.points);
#endif
}
