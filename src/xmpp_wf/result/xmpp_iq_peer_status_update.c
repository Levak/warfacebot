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
#include <wb_dbus.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void xmpp_iq_peer_status_update_cb(const char *msg_id,
                                          const char *msg,
                                          void *args)
{
    /* Answer
       <iq from='xxxxxxx@warface/GameClient' type='get'>
        <query xmlns='urn:cryonline:k01'>
         <peer_status_update nickname='xxxx' profile_id='xxxx' status='13'
                             experience='xxxx' place_token=''
                             place_info_token=''/>
        </query>
       </iq>
    */

	/* <iq from='22853416@warface/GameClient' to='22910345@warface/GameClient' xml:lang='en' id='uid000000d8' type='get'><query xmlns='urn:cryonline:k01'><peer_status_update nickname='DevilDaga10' profile_id='2367686' status='17' experience='12907188' place_token='@ui_playerinfo_pvproom' place_info_token='@ui_playerinfo_location' mode_info_token='@pvp_tdm_game_mode_desc' mission_info_token='@pvp_mission_display_name_tdm_airbase'/></query></iq>
	*/

	/*
	<iq from='22853416@warface/GameClient' to='22910345@warface/GameClient' xml:lang='en' id='uid0000004a' type='get'><query xmlns='urn:cryonline:k01'><peer_status_update nickname='DevilDaga10' profile_id='2367686' status='17' experience='12907188' place_token='@ui_playerinfo_pveroom' place_info_token='@hardmission' mode_info_token='' mission_info_token=''/></query></iq>
	*/

    if (strstr(msg, "type='result'"))
        return;

    char *jid = get_info(msg, "from='", "'", NULL);
    char *nick = get_info(msg, "nickname='", "'", NULL);
    char *pid = get_info(msg, "profile_id='", "'", NULL);
    int status = get_info_int(msg, "status='", "'", NULL);
    int exp = get_info_int(msg, "experience='", "'", NULL);
	char *place_token = get_info(msg, "place_token='", "'", NULL);
	char *place_info_token = get_info(msg, "place_info_token='", "'", NULL);
	char *mode_info_token = get_info(msg, "mode_info_token='", "'", NULL);
	char *mission_info_token = get_info(msg, "mission_info_token='", "'", NULL);

#ifdef DBUS_API
    dbus_api_emit_status_update(nick, status, exp, 0);
#endif /* DBUS_API */

    if (status == STATUS_OFFLINE || status & STATUS_LEFT)
        friend_list_update(NULL, nick, pid, status, exp, place_token, place_info_token, mode_info_token, mission_info_token);
    else
        friend_list_update(jid, nick, pid, status, exp, place_token, place_info_token, mode_info_token, mission_info_token);

    free(jid);
    free(nick);
    free(pid);
	free(place_token);
	free(place_info_token);
	free(mode_info_token);
	free(mission_info_token);
}

void xmpp_iq_peer_status_update_r(void)
{
    qh_register("peer_status_update", 1, xmpp_iq_peer_status_update_cb, NULL);
}
