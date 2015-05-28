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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void xmpp_iq_invitation_request_cb(const char *msg_id,
                                          const char *msg,
                                          void *args)
{
    /* Accept any invitation
       <iq from='masterserver@warface/pve_12' id='uid0002d87c' type='get'>
        <query xmlns='urn:cryonline:k01'>
         <data query_name='invitation_request' from='XXX'
             ticket='XXXX_XXXX_XXXX' room_id='2416'
             ms_resource='pve_12' is_follow='0'
             compressedData='...' originalSize='2082'/>
        </query>
       </iq>
     */

    char *server = get_info(msg, "from='", "'", "Server");
    char *data = wf_get_query_content(msg);
	
/* 	<invitation_request from='Devil_Daga' ticket='12752_1608478_DevilsBitch1' room_id='12752' ms_resource='pve_11' is_follow='0' group_id='e7e423e0-6831-4a21-932a-e41bc9471b05'>
		<game_room room_id='12752' room_type='1'>
			<core teams_switched='0' room_name='Devil_Daga&apos;s GAME ROOM' private='1' players='1' can_start='0' team_balanced='1' revision='6'>
				<players>
					<player profile_id='1608478' team_id='0' status='0' observer='0' nickname='Devil_Daga' clanName='Noblesse-Oblige' class_id='0' online_id='20543777@warface/GameClient' group_id='e7e423e0-6831-4a21-932a-e41bc9471b05' presence='17' experience='132726' banner_badge='452' banner_mark='4294967295' banner_stripe='282'>
						<skills>
							<skill type='campaignsection3' value='0.6581485'/>
							<skill type='easymission' value='0.6453421'/>
							<skill type='hardmission' value='0.6575465'/>
							<skill type='normalmission' value='0.6577018'/>
							<skill type='trainingmission' value='0.6057903'/>
						</skills>
					</player>
				</players>
				<team_colors>
					<team_color id='1' color='4294907157'/>
					<team_color id='2' color='4279655162'/>
				</team_colors>
			</core>
			<session id='' status='0' game_progress='0' start_time='18446744011573954816' revision='2'/>
			<custom_params friendly_fire='0' enemy_outlines='1' auto_team_balance='0' dead_can_chat='1' join_in_the_process='1' max_players='5' class_restriction='253' inventory_slot='2113929215' revision='4'/>
			<mission mission_key='d1cac6df-53e5-4c87-98cf-af8490d16cfb' no_teams='1' name='@na_mission_path01_1' setting='africa/africa_base' mode='pve' mode_name='@PvE_game_mode_desc' mode_icon='pve_icon' description='@mission_desc_africa_path' image='mapImgAfrica' difficulty='easy' type='trainingmission' time_of_day='9:06' revision='4'>
				<objectives factor='1'>
					<objective id='0' type='primary'/>
					<objective id='12' type='secondary'/>
				</objectives>
			</mission>
			<pings revision='1'/>
			<kick_vote_params success='0.6' timeout='60' cooldown='300' revision='1'/>
			<room_master master='1608478' revision='4'/>
		</game_room>
	</invitation_request>
 */
	
    if (!data)
    {
		return;
	}

	get_info(data, "invitation_request from='", "'", "Invited by\t");
    char *resource = get_info(data, "ms_resource='", "'", NULL);
    char *ticket = get_info(data, "ticket='", "'", NULL);
    char *room = get_info(data, "room_id='", "'", "Room ID:\t");
	get_info(data, "difficulty='", "'", "Difficulty:\t" );
	get_info(data, "room_name='", "'", "Room name:\t" );

    if (server && resource && ticket && room)
    {
        /* 1. Confirm invitation */
        send_stream_format(session.wfs,
                           "<iq to='%s' type='get'>"
                           " <query xmlns='urn:cryonline:k01'>"
                           "  <invitation_accept ticket='%s' result='0'/>"
                           " </query>"
                           "</iq>",
                           server, ticket);

        /* 2. Join the room */
        xmpp_iq_gameroom_join(resource, room);
    }

    free(server);
    free(ticket);
    free(room);
    free(resource);

    free(data);
}

void xmpp_iq_invitation_request_r(void)
{
    qh_register("invitation_request", xmpp_iq_invitation_request_cb, NULL);
}
