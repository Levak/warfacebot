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
#include <wb_mission.h>
#include <wb_cvar.h>
#include <wb_list.h>
#include <wb_masterserver.h>
#include <wb_log.h>
#include <wb_status.h>
#include <wb_item.h>
#include <wb_lang.h>

#include <stdlib.h>
#include <string.h>

struct cb_args
{
    char *channel;
    f_resync_profile_cb cb;
    void *args;
};

static void xmpp_iq_resync_profile_cb(const char *msg,
                                    enum xmpp_msg_type type,
                                    void *args)
{
    /* Answer
      <iq from='masterserver@warface/pve_12' type='result'>
       <query xmlns='urn:cryonline:k01'>
        <resync_profile>

         <item ... />...
         <unlocked_item ... />...

         <money game_money='xxxx'
                cry_money='xxxx'
                crown_money='xxxx' />

         <character nick='xxxx' gender='male' height='1'
                    fatness='0' head='' current_class='0'
                    experience='xxxx' pvp_rating_points='xxx'
                    banner_badge='xxxx' banner_mark='xxxx'
                    banner_stripe='xxxx'>
          <RatingGameBan ... />
         </character>

         <progression>
          <profile_progression .../>
         </progression>

        </resync_profile>
       </query>
      </iq>
     */

    struct cb_args *a = (struct cb_args *) args;

    if (msg == NULL)
    {
        free(a);
        return;
    }

    if (type ^ XMPP_TYPE_ERROR)
    {
        char *data = wf_get_query_content(msg);

        if (data != NULL)
        {
            /* Update experience */
            {
                unsigned int experience =
                    get_info_int(data, "experience='", "'",
                                 LANG(experience));

                if (experience > 0)
                    session.profile.experience = experience;
            }

            /* Update PvP rating points */
            {
                unsigned int rating_points =
                    get_info_int(data, "pvp_rating_points='", "'",
                                 LANG(rating_points));

                if (rating_points > 0)
                    session.profile.stats.pvp.rating_points = rating_points;
            }

            /* Update banner */
            {
                unsigned int banner_badge =
                    get_info_int(data, "banner_badge='", "'", NULL);
                unsigned int banner_mark =
                    get_info_int(data, "banner_mark='", "'", NULL);
                unsigned int banner_stripe =
                    get_info_int(data, "banner_stripe='", "'", NULL);

                if (banner_badge > 0)
                    session.profile.banner.badge = banner_badge;
                if (banner_mark > 0)
                    session.profile.banner.mark = banner_mark;
                if (banner_stripe > 0)
                    session.profile.banner.stripe = banner_stripe;
            }

            /* Update money */
            {
                unsigned int game_money =
                    get_info_int(data, "game_money='", "'",
                                 LANG(money_game));
                unsigned int crown_money =
                    get_info_int(data, "crown_money='", "'",
                                 LANG(money_crown));
                unsigned int cry_money =
                    get_info_int(data, "cry_money='", "'",
                                 LANG(money_cry));

                if (game_money > 0)
                    session.profile.money.game = game_money;
                if (crown_money > 0)
                    session.profile.money.crown = crown_money;
                if (cry_money > 0)
                    session.profile.money.cry = cry_money;
            }

            /* Update current class */
            {
                session.profile.curr_class =
                    get_info_int(data, "current_class='", "'", NULL);
            }

            /* Fetch items */
            {
                const char *m = data;
                struct list *items = item_list_new();

                profile_item_list_free();

                while ((m = strstr(m, "<item")))
                {
                    char *item = get_info(m, "<item", "/>", NULL);

                    char *name = get_info(item, "name='", "'", NULL);

                    struct game_item *i = calloc(1, sizeof (struct game_item));

                    i->id = get_info_int(item, "id='", "'", NULL);
                    i->name = name;
                    i->config = get_info(item, "config='", "'", NULL);

                    i->equipped = get_info_int(item, "equipped='", "'", NULL);
                    i->slot = get_info_int(item, "slot='", "'", NULL);

                    i->attached_to = get_info_int(item, "attached_to='", "'", NULL);
                    i->is_default = get_info_int(item, "default='", "'", NULL);
                    i->permanent = get_info_int(item, "permanent='", "'", NULL);

                    i->quantity =
                        get_info_int(item, "quantity='", "'", NULL);
                    i->expired_confirmed =
                        get_info_int(item, "expired_confirmed='", "'", NULL);
                    i->buy_time_utc =
                        get_info_int(item, "buy_time_utc='", "'", NULL);
                    i->expiration_time_utc =
                        get_info_int(item, "expiration_time_utc='", "'", NULL);
                    i->seconds_left =
                        get_info_int(item, "seconds_left='", "'", NULL);

                    /* Update currently equiped primary weapon */
                    if (i->equipped &&
                        (i->slot == (1 << (5 * session.profile.curr_class))))
                    {
                        free(session.profile.primary_weapon);
                        session.profile.primary_weapon =
                            i->name ? strdup(i->name) : NULL;
                    }

                    list_add(items, i);

                    free(item);
                    ++m;
                }

                profile_item_list_init(items);
            }

            /* Fetch unlocked items */
            {
                int unlocked_items = 0;
                const char *m = data;

                while ((m = strstr(m, "<unlocked_item")))
                {
                    char *uitem = get_info(m, "<unlocked_item", "/>", NULL);
                    unsigned int uitem_id = get_info_int(uitem, "id='", "'", NULL);
                    const struct game_item *i =
                        item_list_get_by_id(session.wf.items.list, uitem_id);

                    if (i != NULL && i->locked != 0)
                    {
                        ++unlocked_items;
                    }

                    free(uitem);
                    ++m;
                }

                if (unlocked_items > 0)
                {
                    session.profile.stats.items_unlocked = unlocked_items;
                    xprintf("%s: %d/%d",
                            LANG(unlocked_items),
                            session.profile.stats.items_unlocked,
                            session.wf.total_locked_items);
                }
            }
        }

        if (a->cb)
            a->cb(a->args);

        free(data);
    }

    free(a);
}

void xmpp_iq_resync_profile(f_resync_profile_cb f, void *args)
{
    struct cb_args *a = calloc(1, sizeof (struct cb_args));

    a->cb = f;
    a->args = args;

    xmpp_send_iq_get(
        JID_MS(session.online.channel),
        xmpp_iq_resync_profile_cb, a,
        "<query xmlns='urn:cryonline:k01'>"
        "<resync_profile/>"
        "</query>",
        NULL);
}
