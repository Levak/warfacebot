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

#include <wb_log.h>
#include <stdlib.h>

void xmpp_iq_broadcast_session_result_cb(const char *msg_id,
                                         const char *msg,
                                         void *args)
{
    /* Answer:
       <iq from='masterserver@warface/pve_11' type='get'>
        <query xmlns='urn:cryonline:k01'>
         <broadcast_session_result bcast_receivers="xx@warface/GameClient,...">
          <player_result nickname="xxxx" experience="xxxx"
             pvp_rating_points="0" money="xxxx" gained_crown_money="0"
             no_crown_rewards="1" sponsor_points="xxxx"

             bonus_experience="0" bonus_money="0"
             bonus_sponsor_points="0"

             experience_boost="0"
             money_boost="0" sponsor_points_boost="0"
             experience_boost_percent="0" money_boost_percent="0"
             sponsor_points_boost_percent="0"

             completed_stages="0" is_vip="0" score="xxxx"
             dynamic_multipliers_info="" dynamic_crown_multiplier="1">
           <profile_progression_update profile_id="xxxxxxx"
              mission_unlocked="none,trainingmission,easymission,..."
              tutorial_unlocked="7" tutorial_passed="7" class_unlocked="29"/>
          </player_result>
          ...
         </broadcast_session_result>
        </query>
      </iq>
    */

    char *data = wf_get_query_content(msg);

    if (data == NULL)
        return;

    const char *m = data;

    while ((m = strstr(m, "<player_result")) != NULL)
    {
        char *results = get_info(m, "<player_result", "</player_result>", NULL);
        char *nickname = get_info(results, "nickname='", "'", NULL);

        if (strcmp(nickname, session.profile.nickname) == 0)
        {
            unsigned experience = get_info_int(results, "experience='", "'", NULL);
            unsigned wf_money = get_info_int(results, "money='", "'", NULL);
            unsigned crown_money =
                get_info_int(results, "gained_crown_money='", "'", NULL);
            unsigned pvp_rating_points =
                get_info_int(results, "pvp_rating_points='", "'", NULL);;

            session.profile.experience += experience;
            session.profile.money.game += wf_money;
            session.profile.money.crown += crown_money;

            if (pvp_rating_points
                != session.profile.stats.pvp.rating_points)
            {
                session.profile.stats.pvp.rating_points =
                    pvp_rating_points;

                xprintf("Updated PvP rating points: %u\n",
                        pvp_rating_points);
            }

            xprintf("Status after results: "
                    "XP:%u (+%u)\tMoney:%u (+%u)\tCrowns:%u (+%u)\n",
                    session.profile.experience, experience,
                    session.profile.money.game, wf_money,
                    session.profile.money.crown, crown_money);
        }

        free(nickname);
        free(results);
        ++m;
    }

    free(data);
}

void xmpp_iq_broadcast_session_result_r(void)
{
    qh_register("broadcast_session_result", 1, xmpp_iq_broadcast_session_result_cb, NULL);
}
