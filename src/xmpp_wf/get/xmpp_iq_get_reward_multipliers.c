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
#include <wb_lang.h>

#include <stdlib.h>

struct cb_args
{
    f_id_callback cb;
    void *args;
};

static void xmpp_iq_get_reward_multipliers_cb(const char *msg,
                                        enum xmpp_msg_type type,
                                        void *args)
{
    /* Answer:
       <iq from='masterserver@warface/xx' type='result'>
        <query xmlns='urn:cryonline:k01'>
         <get_reward_multipliers
             money_multiplier='1'
             exp_multiplier='1'
             sp_multiplier='1'
             crown_multiplier='1' />
        </query>
       </iq>
    */

    struct cb_args *a = (struct cb_args *) args;

    if (type ^ XMPP_TYPE_ERROR)
    {
        char *data = wf_get_query_content(msg);

        if (data != NULL)
        {
          float money_multiplier = get_info_float(data, "money_multiplier='", "'", NULL);
          float exp_multiplier = get_info_float(data, "exp_multiplier='", "'", NULL);
          float sp_multiplier = get_info_float(data, "sp_multiplier='", "'", NULL);
          float crown_multiplier = get_info_float(data, "crown_multiplier='", "'", NULL);

          xprintf("%s: x%.1f %s | x%.1f %s | x%.1f %s | x%.1f %s",
                  LANG(console_multipliers),
                  money_multiplier, LANG(money_game_short),
                  exp_multiplier, LANG(experience_short),
                  sp_multiplier, LANG(sponsor_points_short),
                  crown_multiplier, LANG(money_crown_short));
        }

        free(data);
    }

    if (a->cb)
        a->cb(msg, type, a->args);

    free(a);
}

void xmpp_iq_get_reward_multipliers(f_id_callback cb,
                                    void *args)
{
    struct cb_args *a = calloc(1, sizeof (struct cb_args));

    a->cb = cb;
    a->args = args;

    xmpp_send_iq_get(
        JID_MS(session.online.channel),
        xmpp_iq_get_reward_multipliers_cb, a,
        "<query xmlns='urn:cryonline:k01'>"
        "<get_reward_multipliers/>"
        "</query>",
        NULL);
}
