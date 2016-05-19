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
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>
#include <wb_session.h>
#include <wb_friend.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

static void peer_player_info_viewer_cb ( const char *info, void *args )
{
	char *nickname = get_info ( info, "nickname='", "'", NULL );
	if ( list_get ( session.profile.friends, nickname ) )
		LOGPRINT ( "%-20s " KGRN BOLD "%s\n", "PROFILE VIEWED BY", nickname );
	else
		LOGPRINT ( "%-20s " KBLU BOLD "%s\n", "PROFILE VIEWED BY", nickname );

	free ( nickname );
}

static void xmpp_iq_peer_player_info_cb ( const char *msg_id,
										  const char *msg,
										  void *args )
{
	/* Answer
	   <iq from="xxx@warface/GameClient" id="xxxxxx" type="get">
		<query xmlns="urn:cryonline:k01">
		 <peer_player_info/>
		</query>
	   </iq>
	*/

	char *jid = get_info ( msg, "from='", "'", NULL );
	char *clan_stats;

	xmpp_iq_peer_player_info ( jid, peer_player_info_viewer_cb, NULL );

	if ( session.clan.id != 0 )
	{
		FORMAT ( clan_stats,
				 "    clan_name='%s' clan_role='%i'"
				 "    clan_position='%i' clan_points='%i'"
				 "    clan_member_since='%X'",
				 session.clan.name,
				 session.clan.role,
				 session.clan.own_position,
				 session.clan.points,
				 session.clan.joined );
	}
	else
		clan_stats = strdup ( "" );

	send_stream_format ( session.wfs,
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
						 jid, msg_id, session.xmpp.jid, session.profile.nickname,
						 "ar03_bundle_shop",
						 (unsigned) -1, (unsigned) -1,
						 (unsigned) -1, session.profile.experience,
						 (unsigned) ( min ( 111, session.profile.experience / 10000 ) ) /* items_unlocked */,
						 (unsigned) ( 10 + session.profile.experience / 60000 ) /* challenges_completed */,
						 (unsigned) ( session.profile.experience / 3000 ) /* missions_completed */,
						 (unsigned) ( session.profile.experience / 8200 ) /* pvp_wins */,
						 (unsigned) ( session.profile.experience / 9000 ) /* pvp_loses */,
						 (unsigned) ( session.profile.experience / 820 ) /* pvp_kills */,
						 (unsigned) ( session.profile.experience / 900 ) /* pvp_deaths */,
						 (unsigned) ( session.profile.experience * 1.5 ) /* playtime_seconds */,
						 1.0f / ( (float) rand ( ) ) + 1.0f /* leavings_percentage */,
						 (unsigned) ( session.profile.experience / 5000 ) /* coop_climbs_performed */,
						 (unsigned) ( session.profile.experience / 5000 ) /* coop_assists_performe */,
						 clan_stats );

	free ( clan_stats );
	free ( jid );
}

void xmpp_iq_peer_player_info_r ( void )
{
	qh_register ( "peer_player_info", 1, xmpp_iq_peer_player_info_cb, NULL );
}