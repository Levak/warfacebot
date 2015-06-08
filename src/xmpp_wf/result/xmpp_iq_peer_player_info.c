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
#include <wb_session.h>
#include <wb_friend.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void xmpp_iq_peer_player_info_cb(const char *msg_id,
                                        const char *msg,
                                        void *args)
{
    /* Answer
       <iq from="xxx@warface/GameClient" id="xxxxxx" type="get">
        <query xmlns="urn:cryonline:k01">
         <peer_player_info/>
        </query>
       </iq>
    */

    if (xmpp_is_error(msg) || strstr(msg, "type='result'"))
        return;

    char *jid = get_info(msg, "from='", "'", NULL);

    char *clan_stats;
    if (session.clan_id != 0)
    {
        FORMAT(clan_stats,
               "    clan_name='%s' clan_role='%i'"
               "    clan_position='%i' clan_points='%i'"
               "    clan_member_since='%X'",
               session.clan_name,
               session.clan_role,
               1 /* TODO: session.clan_own_position */,
               session.clan_points,
               session.clan_joined);
    }
    else
        clan_stats = strdup("");

    send_stream_format(session.wfs,
                       "<iq to='%s' id='%s' type='result'>"
                       " <query xmlns='urn:cryonline:k01'>"
                       "  <peer_player_info online_id='%s' nickname='%s'"
                       "    primary_weapon='%s'"
                       "    banner_badge='%u' banner_mark='%u'"
                       "    banner_stripe='%u' experience='%u'"
                       "    items_unlocked='%u'"
                       "    challenges_completed='%u'"
                       "    missions_completed='%u'"
                       "    pvp_wins='%u' pvp_loses='%u'"
                       "    pvp_kills='%u' pvp_deaths='%u'"
                       "    playtime_seconds='%u'"
                       "    leavings_percentage='%f'"
                       "    coop_climbs_performed='%u'"
                       "    coop_assists_performed='%u'"
                       "    favorite_pvp_class='0' favorite_pve_class='0'"
                       "    %s />"
                       " </query>"
                       "</iq>",
                       jid, msg_id, session.jid, session.nickname,
                       "ar03_bundle_shop",
                       4294967295, 4294967295,
                       4294967295, session.experience,
                       (unsigned) (rand() % 10 + 10) /* items_unlocked */,
                       (unsigned) (rand() % 10 + 10) /* challenges_completed */,
                       (unsigned) (rand() % 10 + 10) /* missions_completed */,
                       (unsigned) (rand() % 10 + 10) /* pvp_wins */,
                       (unsigned) (rand() % 10 + 10) /* pvp_loses */,
                       (unsigned) (rand() % 10 + 10) /* pvp_kills */,
                       (unsigned) (rand() % 10 + 10) /* pvp_deaths */,
                       (unsigned) (rand() % 10000 + 10000) /* playtime_seconds */,
                       1.0f / ((float) rand()) + 1.0f /* leavings_percentage */,
                       (unsigned) (rand() % 10 + 10) /* coop_climbs_performed */,
                       (unsigned) (rand() % 10 + 10) /* coop_assists_performe */,
                       clan_stats);

    free(clan_stats);
    free(jid);
}

void xmpp_iq_peer_player_info_r(void)
{
    qh_register("peer_player_info", 1, xmpp_iq_peer_player_info_cb, NULL);
}
