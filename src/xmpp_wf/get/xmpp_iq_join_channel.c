/**
 * WarfaceBot, a blind XMPP client for Warface (FPS)
 * Copyright (C) 2015, 2016 Levak Borok <levak92@gmail.com>
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
#include <wb_stream.h>
#include <wb_session.h>
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>
#include <wb_game.h>
#include <wb_mission.h>

#include <stdio.h>
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
      <iq from='masterserver@warface/pve_12' to='xxxxxx@warface/GameClient' type='result'>
       <query xmlns='urn:cryonline:k01'>
        <data query_name='join_channel' compressedData='...' originalSize='13480'/>
       </query>
      </iq>
     */

    struct cb_args *a = (struct cb_args *) args;

    if (type & XMPP_TYPE_ERROR)
    {
        fprintf(stderr, "Failed to join channel\nReason: ");

        int code = get_info_int(msg, "code='", "'", NULL);
        int custom_code = get_info_int(msg, "custom_code='", "'", NULL);

        switch (code)
        {
            case 1006:
                fprintf(stderr, "QoS limit reached\n");
                break;
            case 503:
                fprintf(stderr, "Invalid channel (%s)\n", a->channel);
                break;
            case 8:
                switch (custom_code)
                {
                    case 0:
                        fprintf(stderr, "Invalid token (%s) or userid (%s)\n",
                                session.online.active_token,
                                session.online.id);
                        break;
                    case 1:
                        fprintf(stderr, "Invalid profile_id (%s)\n",
                                session.profile.id);
                        break;
                    case 2:
                        fprintf(stderr, "Game version mismatch (%s)\n",
                                game_version_get());
                        break;
                    case 3:
                        fprintf(stderr, "Banned\n");
                        break;
                    case 5:
                        fprintf(stderr, "Rank restricted\n");
                        break;
                    default:
                        fprintf(stderr, "Unknown code (%d)\n", custom_code);
                        break;
                }
                break;
            default:
                fprintf(stderr, "Unknown\n");
                break;
        }
    }
    else
    {
        char *data = wf_get_query_content(msg);

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
            }

            /* Update experience */
            {
                unsigned int experience =
                    get_info_int(data, "experience='", "'", "EXPERIENCE");

                if (experience > 0)
                    session.profile.experience = experience;
            }

            /* Update PvP rating points */
            {
                unsigned int rating_points =
                    get_info_int(data, "pvp_rating_points='", "'", NULL);

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
                    get_info_int(data, "game_money='", "'", "MONEY");
                unsigned int crown_money =
                    get_info_int(data, "crown_money='", "'", "CROWNS");
                unsigned int cry_money =
                    get_info_int(data, "cry_money='", "'", "KREDITS");

                if (game_money > 0)
                    session.profile.money.game = game_money;
                if (crown_money > 0)
                    session.profile.money.crown = crown_money;
                if (cry_money > 0)
                    session.profile.money.cry = cry_money;
            }

            /* Fetch currently equipped weapon */
            {
                const char *m = data;
                enum e_class class = get_info_int(data, "current_class='", "'", NULL);

                while ((m = strstr(m, "<item")))
                {
                    char *item = get_info(m, "<item", "/>", NULL);
                    int equipped = get_info_int(item, "equipped='", "'", NULL);
                    int slot = get_info_int(item, "slot='", "'", NULL);

                    if (equipped && (slot == (1 << class)))
                    {
                        char *name = get_info(item, "name='", "'", NULL);
                        free(session.profile.primary_weapon);
                        session.profile.primary_weapon = name;
                    }

                    free(item);
                    ++m;
                }
            }

            /* Fetch unlocked items */
            {
                unsigned int unlocked_items = 0;
                const char *m = data;

                while ((m = strstr(m, "<unlocked_item")))
                {
                    ++unlocked_items;
                    ++m;
                }

                if (unlocked_items > 0)
                    session.profile.stats.items_unlocked = unlocked_items;
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
        }

        /* Update stats */
        xmpp_iq_get_player_stats(NULL, NULL);
        xmpp_iq_get_achievements(session.profile.id, NULL, NULL);

        /* Ask for today's missions list */
        mission_list_update(NULL, NULL);

        /* Inform to k01 our status */
        xmpp_iq_player_status(STATUS_ONLINE | STATUS_LOBBY);

        if (a->cb)
            a->cb(a->args);

        free(data);
        free(a->channel);
        free(a);
    }
}

void xmpp_iq_join_channel(const char *channel, f_join_channel_cb f, void *args)
{
    int is_switch = session.profile.status >= STATUS_LOBBY;
    struct cb_args *a = calloc(1, sizeof (struct cb_args));

    a->cb = f;
    a->args = args;

    if (channel == NULL)
        channel = session.online.channel;

    if (channel)
        a->channel = strdup(channel);
    else
        a->channel = NULL;

    t_uid id;

    idh_generate_unique_id(&id);
    idh_register(&id, 0, xmpp_iq_join_channel_cb, a);

    /* Join CryOnline channel */
    send_stream_format(session.wfs,
                       "<iq id='%s' to='k01.warface' type='get'>"
                       "<query xmlns='urn:cryonline:k01'>"
                       "<%s_channel version='%s' token='%s' region_id='global'"
                       "     profile_id='%s' user_id='%s' resource='%s'"
                       "     user_data='' hw_id='' build_type='--release'/>"
                       "</query>"
                       "</iq>",
                       &id, is_switch ? "switch" : "join",
                       game_version_get(),
                       session.online.active_token,
                       session.profile.id,
                       session.online.id,
                       a->channel);
}
