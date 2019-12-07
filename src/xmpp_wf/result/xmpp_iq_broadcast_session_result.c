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
#include <wb_list.h>
#include <wb_log.h>
#include <wb_dbus.h>
#include <wb_lang.h>

#include <stdlib.h>

static int player_result_cmp(struct player_result *pr1, struct player_result *pr2)
{
    return strcmp(pr1->nickname, pr2->nickname);
}

static void player_result_free(struct player_result *pr)
{
    free(pr->nickname);
    free(pr->mission_unlocked);
    free(pr);
}


void xmpp_iq_broadcast_session_result_cb(const char *msg_id,
                                         const char *msg,
                                         void *args)
{
    /* Answer:
       <iq from='masterserver@warface/pve_11' type='get'>
        <query xmlns='urn:cryonline:k01'>
         <broadcast_session_result bcast_receivers="xx@warface/GameClient,...">
          <player_result nickname="xxxx" experience="xxxx"
             pvp_rating_rank="0" money="xxxx" gained_crown_money="0"
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
    struct list *l = list_new(
        (f_list_cmp) player_result_cmp,
        (f_list_free) player_result_free);

    while ((m = strstr(m, "<player_result")) != NULL)
    {
        struct player_result *pr = calloc(1, sizeof (struct player_result));
        char *results = get_info(m, "<player_result", "</player_result>", NULL);

        pr->nickname = get_info(results, "nickname='", "'", NULL);
        pr->experience = get_info_int(results, "experience='", "'", NULL);
        pr->money = get_info_int(results, "money='", "'", NULL);
        pr->gained_crown_money =
            get_info_int(results, "gained_crown_money='", "'", NULL);
        pr->pvp_rating_points =
            get_info_int(results, "pvp_rating_rank='", "'", NULL);;

        pr->mission_unlocked =
            get_info(results, "mission_unlocked='", "'", NULL);
        pr->class_unlocked =
            get_info_int(results, "class_unlocked='", "'", NULL);
        pr->tutorial_unlocked =
            get_info_int(results, "tutorial_unlocked='", "'", NULL);
        pr->tutorial_passed =
            get_info_int(results, "tutorial_passed='", "'", NULL);

        if (0 == strcmp(pr->nickname, session.profile.nickname))
        {
            session.profile.experience += pr->experience;
            session.profile.money.game += pr->money;
            session.profile.money.crown += pr->gained_crown_money;

            if (pr->pvp_rating_points
                != session.profile.stats.pvp.rating_points)
            {
                session.profile.stats.pvp.rating_points =
                    pr->pvp_rating_points;

                xprintf("%s: %u",
                        LANG(update_rating_points),
                        pr->pvp_rating_points);
            }

            xprintf("%s: "
                    "%s:%u (+%u)\t%s:%u (+%u)\t%s:%u (+%u)",
                    LANG(update_profile_status),
                    LANG(experience),
                    session.profile.experience, pr->experience,
                    LANG(money_game),
                    session.profile.money.game, pr->money,
                    LANG(money_crown),
                    session.profile.money.crown, pr->gained_crown_money);

            free(session.profile.progression.mission_unlocked);
            if (pr->mission_unlocked != NULL)
                session.profile.progression.mission_unlocked =
                    strdup(pr->mission_unlocked);
            session.profile.progression.class_unlocked =
                pr->class_unlocked;
            session.profile.progression.tutorial_unlocked =
                pr->tutorial_unlocked;
            session.profile.progression.tutorial_passed =
                pr->tutorial_passed;

#ifdef DBUS_API
            dbus_api_emit_profile_progression(
                session.profile.progression.mission_unlocked,
                session.profile.progression.class_unlocked,
                session.profile.progression.tutorial_unlocked,
                session.profile.progression.tutorial_passed);
#endif /* DBUS_API */
        }

        list_add(l, pr);
        free(results);
        ++m;
    }

#ifdef DBUS_API
    dbus_api_emit_match_results(l);
#endif /* DBUS_API */

    list_free(l);
    free(data);
}

void xmpp_iq_broadcast_session_result_r(void)
{
    qh_register("broadcast_session_result", 1, xmpp_iq_broadcast_session_result_cb, NULL);
}
