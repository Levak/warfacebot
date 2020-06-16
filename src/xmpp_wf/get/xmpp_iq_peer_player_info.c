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
    f_peer_player_info_cb cb;
    void *args;
};

static void xmpp_iq_peer_player_info_cb(const char *msg,
                                        enum xmpp_msg_type type,
                                        void *args)
{
    /* Answer:
       <iq type='result' to='xxxxx@warface/GameClient'>
        <query xmlns='urn:cryonline:k01'>
         <peer_player_info
            online_id='xxxx@warface/GameClient'
            nickname=''

            primary_weapon='mg08_camo02_shop'
            primary_weapon_skin=''
            banner_badge=''
            banner_mark=''
            banner_stripe=''
            experience=''
            pvp_rating=''
            pvp_rating_rank=''
            items_unlocked=''
            challenges_completed=''
            missions_completed=''
            pvp_wins=''
            pvp_loses=''
            pvp_kills=''
            pvp_deaths=''
            playtime_seconds=''
            leavings_percentage=''
            coop_climbs_performed=''
            coop_assists_performed=''
            favorite_pvp_class=''
            favorite_pve_class=''

            clan_name=''
            clan_position=''
            clan_member_since=''
         />
        </query>
       </iq>
    */

    struct cb_args *a = (struct cb_args *) args;

    if (type & XMPP_TYPE_ERROR)
    {
        if (a->cb)
            a->cb(NULL, a->args);
    }
    else
    {
        struct player_peer_info ppi;

        ppi.online_id = get_info(msg, "online_id='", "'", NULL);
        ppi.nickname = get_info(msg, "nickname='", "'", NULL);
        ppi.primary_weapon = get_info(msg, "primary_weapon='", "'", NULL);
        ppi.primary_weapon_skin = get_info(msg, "primary_weapon_skin='", "'", NULL);

        ppi.banner_badge = get_info_int(msg, "banner_badge='", "'", NULL);
        ppi.banner_mark = get_info_int(msg, "banner_mark='", "'", NULL);
        ppi.banner_stripe = get_info_int(msg, "banner_stripe='", "'", NULL);
        ppi.experience = get_info_int(msg, "experience='", "'", NULL);

        ppi.pvp_rating_rank = get_info_int(msg, "pvp_rating_rank='", "'", NULL);
        ppi.items_unlocked = get_info_int(msg, "items_unlocked='", "'", NULL);
        ppi.challenges_completed = get_info_int(msg, "challenges_completed='", "'", NULL);
        ppi.missions_completed = get_info_int(msg, "missions_completed='", "'", NULL);

        ppi.pvp_wins = get_info_int(msg, "pvp_wins='", "'", NULL);
        ppi.pvp_loses = get_info_int(msg, "pvp_loses='", "'", NULL);
        ppi.pvp_total_matches = get_info_int(msg, "pvp_total_matches='", "'", NULL);
        ppi.pvp_kills = get_info_int(msg, "pvp_kills='", "'", NULL);
        ppi.pvp_deaths = get_info_int(msg, "pvp_deaths='", "'", NULL);

        ppi.playtime_seconds = get_info_int(msg, "playtime_seconds='", "'", NULL);
        ppi.leavings_percentage = get_info_int(msg, "leavings_percentage='", "'", NULL);

        ppi.coop_climbs_performed = get_info_int(msg, "coop_climbs_performed='", "'", NULL);
        ppi.coop_assists_performed = get_info_int(msg, "coop_assists_performed='", "'", NULL);
        ppi.favorite_pvp_class = get_info_int(msg, "favorite_pvp_class='", "'", NULL);
        ppi.favorite_pve_class = get_info_int(msg, "favorite_pve_class='", "'", NULL);

        ppi.clan_name = get_info(msg, "clan_name='", "'", NULL);
        ppi.clan_role = get_info_int(msg, "clan_role='", "'", NULL);
        ppi.clan_position = get_info_int(msg, "clan_position='", "'", NULL);
        ppi.clan_points = get_info_int(msg, "clan_points='", "'", NULL);
        ppi.clan_member_since = get_info(msg, "clan_member_since='", "'", NULL);

        if (a->cb)
            a->cb(&ppi, a->args);

        free(ppi.online_id);
        free(ppi.nickname);
        free(ppi.primary_weapon);
        free(ppi.primary_weapon_skin);

        free(ppi.clan_name);
        free(ppi.clan_member_since);
    }

    free(a);
}

void xmpp_iq_peer_player_info(const char *online_id,
                              f_peer_player_info_cb f, void *args)
{
    struct cb_args *a = calloc(1, sizeof (struct cb_args));

    a->cb = f;
    a->args = args;

    xmpp_send_iq_get(
        JID(online_id),
        xmpp_iq_peer_player_info_cb, a,
        "<query xmlns='urn:cryonline:k01'>"
        "<peer_player_info/>"
        "</query>",
        NULL);
}
