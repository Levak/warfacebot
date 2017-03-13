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
            pvp_rating_points=''
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
        char *info = get_info(msg, "<peer_player_info", "/>", NULL);

        if (a->cb)
            a->cb(info, a->args);

        free(info);
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
