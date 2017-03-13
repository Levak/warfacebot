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
#include <wb_xmpp_wf.h>

#include <stdlib.h>
#include <string.h>

struct cb_args
{
    f_id_callback cb;
    void *args;
};

static void xmpp_iq_get_player_stats_cb(const char *msg,
                                        enum xmpp_msg_type type,
                                        void *args)
{
    /* Answer:
       <iq from='masterserver@warface/xx' type='result'>
        <query xmlns='urn:cryonline:k01'>
         <get_player_stats>
          <stat stat="player_online_time" Value="21863447"/>
          <stat stat="player_gained_money" Value="280358"/>
          <stat stat="player_max_session_time" Value="9000161"/>
          <stat stat="player_damage" Value="160778"/>
          <stat stat="player_max_damage" Value="1275"/>
          <stat stat="player_heal" Value="406"/>
          <stat mode="PVE" stat="player_sessions_left" Value="2"/>
          <stat mode="PVE" stat="player_kill_streak" Value="114"/>
          <stat mode="PVE" stat="player_kills_ai" Value="525"/>
          <stat mode="PVE" stat="player_deaths" Value="3"/>
          <stat class="Medic" mode="PVE" stat="player_shots" Value="1261"/>
          <stat class="Medic" mode="PVE" stat="player_hits" Value="332"/>
          ...
         </get_player_stats>
        </query>
       </iq>
    */

    struct cb_args *a = (struct cb_args *) args;

    if (type ^ XMPP_TYPE_ERROR)
    {
        char *data = wf_get_query_content(msg);

        if (data != NULL)
        {
            /* Fetch stats, one by one */
            {
                long long int play_time = 0;
                unsigned int missions_completed = 0;
                unsigned int sessions_left = 0;
                unsigned int sessions_count = 0;

                unsigned int pvp_wins = 0;
                unsigned int pvp_loses = 0;
                unsigned int pvp_kills = 0;
                unsigned int pvp_deaths = 0;

                const char *m = data;

                while ((m = strstr(m, "<stat ")))
                {
                    char *stat = get_info(m, "<stat ", "/>", NULL);
                    long long int v = get_info_int(stat, "Value='", "'", NULL);

                    if (strstr(stat, "'player_playtime'") != NULL)
                    {
                        play_time += v / 10;
                    }

                    else if (strstr(stat, "'player_sessions_won'") != NULL)
                    {
                        if (strstr(stat, "PVE") != NULL)
                        {
                            missions_completed += v;
                        }
                        else if (strstr(stat, "PVP") != NULL)
                        {
                            pvp_wins += v;
                        }

                        sessions_count += v;
                    }

                    else if (strstr(stat, "'player_sessions_left'") != NULL)
                    {
                        sessions_left += v;
                        sessions_count += v;
                    }

                    else if (strstr(stat, "'player_sessions_lost'") != NULL)
                    {
                        if (strstr(stat, "PVP") != NULL)
                        {
                            pvp_loses += v;
                        }

                        sessions_count += v;
                    }

                    else if (strstr(stat, "'player_kills_player'") != NULL)
                    {
                        if (strstr(stat, "PVP") != NULL)
                        {
                            pvp_kills += v;
                        }
                    }

                    else if (strstr(stat, "'player_deaths'") != NULL)
                    {
                        if (strstr(stat, "PVP") != NULL)
                        {
                            pvp_deaths += v;
                        }
                    }

                    else if (strstr(stat, "'player_climb_coops'") != NULL)
                    {
                        session.profile.stats.coop_climbs_performed = v;
                    }

                    else if (strstr(stat, "'player_climb_assists'") != NULL)
                    {
                        session.profile.stats.coop_assists_performed = v;
                    }

                    free(stat);
                    ++m;
                }

                if (play_time > 0)
                    session.profile.stats.playtime_seconds = play_time;
                if (missions_completed > 0)
                    session.profile.stats.pve.missions_completed = missions_completed;
                if (sessions_count > 0)
                    session.profile.stats.leavings_percentage =
                        sessions_left / (float) sessions_count;

                if (pvp_wins > 0)
                    session.profile.stats.pvp.wins = pvp_wins;
                if (pvp_loses > 0)
                    session.profile.stats.pvp.loses = pvp_loses;
                if (pvp_kills > 0)
                    session.profile.stats.pvp.kills = pvp_kills;
                if (pvp_deaths > 0)
                    session.profile.stats.pvp.deaths = pvp_deaths;
            }
        }

        free(data);
    }

    if (a->cb)
        a->cb(msg, type, a->args);

    free(a);
}

void xmpp_iq_get_player_stats(f_id_callback cb, void *args)
{
    struct cb_args *a = calloc(1, sizeof (struct cb_args));

    a->cb = cb;
    a->args = args;

    xmpp_send_iq_get(
        JID_MS(session.online.channel),
        xmpp_iq_get_player_stats_cb, a,
        "<query xmlns='urn:cryonline:k01'>"
        "<get_player_stats/>"
        "</query>",
        NULL);
}
