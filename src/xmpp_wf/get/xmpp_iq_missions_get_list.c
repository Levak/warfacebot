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
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>
#include <wb_list.h>
#include <wb_mission.h>

#include <string.h>

struct cb_args
{
    f_list_cb fun;
    void *args;
};

static void xmpp_iq_missions_get_list_cb(const char *msg, void *args)
{
    /* Answer:
       <iq to='masterserver@warface/pve_2' type='get'>
        <query xmlns='urn:cryonline:k01'>
         <data query_name='missions_get_list' compressedData='...'
               originalSize='42'/>
        </query>
       </iq>
     */

    struct cb_args *a = (struct cb_args *) args;

    if (xmpp_is_error(msg))
    {
        free(a);
        return;
    }

    char *data = wf_get_query_content(msg);

    if (!data)
    {
        free(a);
        return;
    }

    /* Content:
      <missions_get_list>
       <mission mission_key='0a88e814-12d9-4b49-ad4a-00416761a87b'
                no_teams='1' name='@snow_mission_survival_02'
                setting='survival/snow_fortress_base' mode='pve'
                mode_name='@PvE_game_mode_desc' mode_icon='pve_icon'
                description='@snow_survival_mission_desc_02'
                image='mapImgSurvivalSnow2' difficulty='survival'
                type='campaignsection2' time_of_day='11:30'>
        <objectives factor='1'>
         <objective id='0' type='primary'/>
        </objectives>
       </mission>
       ...
       <mission mission_key="b9b44f56-c693-4eea-af08-31cddc0402b7"
                no_teams="1" name="@ct_mission_easy05_2"
                setting="china/china_base" mode="pve"
                mode_name="@PvE_game_mode_desc" mode_icon="pve_icon"
                description="@mission_desc_china_j05"
                image="mapImgCTj05_normal" difficulty="hard"
                type="hardmission" time_of_day="9:06">
        <objectives factor="2">
         <objective id="0" type="primary"/>
         <objective id="13" type="secondary"/>
         <objective id="17" type="secondary"/>
        </objectives>
        <CrownRewardsThresholds>
         <TotalPerformance bronze="29595" silver="41855" gold="120220"/>
         <Time bronze="4193094" silver="4193334" gold="4193374"/>
        </CrownRewardsThresholds>
        <CrownRewards bronze="7" silver="19" gold="34"/>
       </mission>
       ...
      </missions_get_list>
     */

    struct list *mission_list = mission_list_new();

    const char *m = strstr(data, "<missions_get_list");

    if (m != NULL)
    {
        m += sizeof ("<missions_get_list");

        while ((m = strstr(m, "<mission")))
        {
            char *ms = get_info(m, "<mission ", "</mission>", NULL);
            m += sizeof ("<mission");

            struct mission *mi = calloc(1, sizeof (struct mission));

            mi->mission_key = get_info(ms, "mission_key='", "'", NULL);
            mi->no_team = get_info_int(ms, "no_team='", "'", NULL);
            mi->name = get_info(ms, "name='", "'", NULL);
            mi->setting = get_info(ms, "setting='", "'", NULL);
            mi->mode = get_info(ms, "mode='", "'", NULL);
            mi->mode_name = get_info(ms, "mode_name='", "'", NULL);
            mi->mode_icon = get_info(ms, "mode_icon='", "'", NULL);
            mi->description = get_info(ms, "description='", "'", NULL);
            mi->image = get_info(ms, "image='", "'", NULL);
            mi->difficulty = get_info(ms, "difficulty='", "'", NULL);
            mi->type = get_info(ms, "type='", "'", NULL);
            mi->time_of_day = get_info(ms, "time_of_day='", "'", NULL);

            char *c_reward = get_info(ms, "<CrownRewards ", ">", NULL);
            {
                if (c_reward != NULL)
                    mi->crown_reward_gold = get_info_int(c_reward, "gold='", "'", NULL);

                free(c_reward);
            }

            char *c_perf = get_info(ms, "<TotalPerformance ", ">", NULL);
            {
                if (c_perf != NULL)
                    mi->crown_perf_gold = get_info_int(c_perf, "gold='", "'", NULL);

                free(c_perf);
            }

            char *c_time = get_info(ms, "<Time ", ">", NULL);
            {
                if (c_time != NULL)
                {
                    mi->crown_time_gold = get_info_int(c_time, "gold='", "'", NULL);
                    mi->crown_time_gold = (1 << 22) - mi->crown_time_gold;
                }

                free(c_time);
            }

            list_add(mission_list, mi);
            free(ms);
        }
    }

    if (a->fun != NULL)
        a->fun(mission_list, a->args);

    free(a);
    free(data);
}

void xmpp_iq_missions_get_list(f_list_cb fun, void *args)
{
    struct cb_args *a = calloc(1, sizeof (struct cb_args));
    a->fun = fun;
    a->args = args;

    t_uid id;

    idh_generate_unique_id(&id);
    idh_register(&id, 0, xmpp_iq_missions_get_list_cb, (void *) a);

    send_stream_format(session.wfs,
                       "<iq id='%s' to='masterserver@warface/%s' type='get'>"
                       " <query xmlns='urn:cryonline:k01'>"
                       "  <missions_get_list/>"
                       " </query>"
                       "</iq>",
                       &id, session.channel);
}

