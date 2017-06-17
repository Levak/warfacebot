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
#include <wb_dbus.h>
#include <wb_lang.h>

#include <stdlib.h>
#include <string.h>

struct cb_args
{
    char *channel;
    f_join_channel_cb cb;
    void *args;
};

static void xmpp_iq_join_channel_cb(const char *msg,
                                    enum xmpp_msg_type type,
                                    void *args)
{
    /* Answer
      <iq from='masterserver@warface/pve_12' type='result'>
       <query xmlns='urn:cryonline:k01'>
        <join_channel>
         <character nick='xxxx' gender='male' height='1'
                    fatness='0' head='' current_class='0'
                    experience='xxxx' pvp_rating_points='xxx'
                    banner_badge='xxxx' banner_mark='xxxx'
                    banner_stripe='xxxx' game_money='xxxx'
                    cry_money='xxxx' crown_money='xxxx'>

          <RatingGameBan ban_timeout='0' unban_time='xxxx'/>

          <item ... />...
          <unlocked_item ... />...
          <expired_item ... />...
          <notif ...>
           ...
          </notif>...

          <sponsor_info>
           <sponsor sponsor_id='0'
                    sponsor_points='xxxxx'
                    next_unlock_item=''/>
           ...
          </sponsor_info>

          <login_bonus current_streak='0' current_reward='1'/>

          <chat_channels>
           <chat channel='0'
                 channel_id='xxxxxx'
                 service_id='xxxxxx'/>
          </chat_channels>

          <progression>
           <profile_progression
              profile_id='xxxx' mission_unlocked='xxxx'
              tutorial_unlocked='7' tutorial_passed='7'
              class_unlocked='29'/>
          </progression>

          <variables>
           <item ... />...
          </variables>

         </character>
        </join_channel>
       </query>
      </iq>
     */

    struct cb_args *a = (struct cb_args *) args;
    char *logout_channel = NULL;

    if (msg == NULL)
    {
        free(a->channel);
        free(a);
        return;
    }

    if (type & XMPP_TYPE_ERROR)
    {
        const char *reason = NULL;

        logout_channel = strdup(a->channel);

        int code = get_info_int(msg, "code='", "'", NULL);
        int custom_code = get_info_int(msg, "custom_code='", "'", NULL);

        switch (code)
        {
            case 1006:
                reason = LANG(error_qos_limit);
                break;
            case 503:
                reason = LANG(error_invalid_channel);
                break;
            case 8:
                switch (custom_code)
                {
                    case 0:
                        reason = LANG(error_invalid_login);
                        break;
                    case 1:
                        reason = LANG(error_invalid_profile);
                        break;
                    case 2:
                        reason = LANG(error_game_version);
                        break;
                    case 3:
                        reason = LANG(error_banned);
                        break;
                    case 5:
                        reason = LANG(error_rank_restricted);
                        break;
                    case 8:
                        reason = LANG(error_already_logged_in);
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }

        if (reason != NULL)
            eprintf("%s (%s)",
                    LANG(error_join_channel),
                    reason);
        else
            eprintf("%s (%i:%i)",
                    LANG(error_join_channel),
                    code,
                    custom_code);
    }
    else
    {
        char *data = wf_get_query_content(msg);

        if (session.online.channel != NULL)
            logout_channel = strdup(session.online.channel);

        /* Leave previous room if any */
        xmpp_iq_gameroom_leave();

        if (data != NULL)
        {
            char is_join_channel = strstr(data, "<join_channel") != NULL;

            /* Update own channel */
            if (a->channel != NULL)
            {
                free(session.online.channel);
                session.online.channel = strdup(a->channel);

                struct masterserver *ms = masterserver_list_get(a->channel);

                free(session.online.channel_type);
                session.online.channel_type = NULL;

                if (ms != NULL)
                {
                    session.online.channel_type = strdup(ms->channel);
                }

#ifdef DBUS_API
                /* Broadcast to DBus */
                dbus_api_emit_channel_update(session.online.channel,
                                             session.online.channel_type);
#endif
                xprintf("%s %s (%s)",
                        LANG(channel_joined),
                        session.online.channel,
                        session.online.channel_type);
            }

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

            if (is_join_channel)
            {
                /* Update money */
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
            if (is_join_channel)
            {
                const char *m = data;
                struct list *items = item_list_new();

                profile_item_list_free();

                while ((m = strstr(m, "<item")))
                {
                    char *item = get_info(m, "<item", "/>", NULL);

                    char *name = get_info(item, "name='", "'", NULL);

                    /* Skip the <variables><item/></variables> nodes */
                    if (name == NULL)
                    {
                        free(item);
                        ++m;
                        continue;
                    }

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

            /* Fetch expired items */
            {
                const char *m = data;

                char *query = strdup("");
                unsigned int expired_items = 0;

                while ((m = strstr(m, "<expired_item")))
                {
                    char *item_id = get_info(m, "id='", "'", NULL);

                    if (item_id == NULL)
                        continue;

                    char *s;
                    FORMAT(s, "%s<item item_id='%s'/>", query, item_id);

                    free(query);
                    query = s;

                    free(item_id);

                    ++expired_items;
                    ++m;
                }

                if (expired_items > 0)
                {
                    {
                        char *s = LANG_FMT(notif_confirm_expiration,
                                           expired_items);
                        xprintf("%s", s);
                        free(s);
                    }

                    xmpp_send_iq_get(
                        JID_MS(session.online.channel),
                        NULL, NULL,
                        "<query xmlns='urn:cryonline:k01'>"
                        "<notify_expired_items>"
                        "%s"
                        "</notify_expired_items>"
                        "</query>",
                        query);
                }

                free(query);
            }

            /* Fetch notifications */
            {
                const char *m = data;

                while ((m = strstr(m, "<notif")))
                {
                    char *notif = get_info(m, "<notif", "</notif>", NULL);

                    xmpp_iq_confirm_notification(notif);
                    free(notif);
                    ++m;
                }
            }

            if (strstr(session.online.channel_type, "pvp") != NULL)
            {
                /* Update quickplay maps */
                xmpp_iq_quickplay_maplist(NULL, NULL);
            }

            /* Update shop */
            xmpp_iq_shop_get_offers(NULL, NULL);

            /* Update stats */
            xmpp_iq_get_player_stats(NULL, NULL);
            xmpp_iq_get_achievements(session.profile.id, NULL, NULL);

            /* Ask for today's missions list */
            mission_list_update(NULL, NULL);

            /* Inform to k01 our status */
            status_set(STATUS_ONLINE | STATUS_LOBBY);



        }

        if (a->cb)
            a->cb(a->args);

        free(data);
    }

    if (logout_channel != NULL
        && session.online.channel != NULL
        && strcmp(session.online.channel, logout_channel) != 0)
    {
        xmpp_send_iq_get(
            JID_MS(logout_channel),
            NULL, NULL,
            "<query xmlns='urn:cryonline:k01'>"
            "<channel_logout/>"
            "</query>",
            NULL);
    }

    free(logout_channel);
    free(a->channel);
    free(a);
}

void xmpp_iq_join_channel(const char *channel,
                          f_join_channel_cb f, void *args)
{
    if (channel == NULL)
        return;

    int is_switch = session.online.status >= STATUS_LOBBY;
    struct cb_args *a = calloc(1, sizeof (struct cb_args));

    a->cb = f;
    a->args = args;
    a->channel = strdup(channel);

    if (is_switch)
    {
        /* Switch to another CryOnline channel */
        xmpp_send_iq_get(
            JID_MS(a->channel),
            xmpp_iq_join_channel_cb, a,
            "<query xmlns='urn:cryonline:k01'>"
            "<switch_channel version='%s' token='%s' region_id='%s'"
            "     profile_id='%s' user_id='%s' resource='%s'"
            "     build_type='--release'/>"
            "</query>",
            cvar.game_version,
            session.online.active_token,
            cvar.online_region_id,
            session.profile.id,
            session.online.id,
            a->channel);
    }
    else
    {
        /* Join CryOnline channel */
        xmpp_send_iq_get(
            JID_K01,
            xmpp_iq_join_channel_cb, a,
            "<query xmlns='urn:cryonline:k01'>"
            "<join_channel"
            " version='%s' token='%s' region_id='%s'"
            " profile_id='%s' user_id='%s' resource='%s'"
            " build_type='--release'"
            " hw_id='%d' os_ver='10' os_64='1'"
            " cpu_vendor='10' cpu_family='10' cpu_model='10'"
            " cpu_stepping='10' cpu_speed='10' cpu_num_cores='1'"
            " gpu_vendor_id='10' gpu_device_id='10'"
            " physical_memory='10'"
            "/>"
            "</query>",
            cvar.game_version,
            session.online.active_token,
            cvar.online_region_id,
            session.profile.id,
            session.online.id,
            a->channel,
            cvar.game_hwid);
    }
}
