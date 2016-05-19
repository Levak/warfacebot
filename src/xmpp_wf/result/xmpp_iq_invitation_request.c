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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void xmpp_iq_invitation_request_cb ( const char *msg_id,
											const char *msg,
											void *args )
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

	/*
	<invitation_request from='DevilDaga10' ticket='14633946711349790664_2367686_DevilsBitch6' room_id='14633946711349790664' ms_resource='pve_8' is_follow='0' group_id='ad4fc6a9-4434-8858-e8277fbfab71'><game_room room_id='14633946711349790664' room_type='1'><core teams_switched='0' room_name='Room' private='1' players='1' can_start='0' team_balanced='1' min_ready_players='3' revision='4'><players><player profile_id='2367686' team_id='1' status='1' observer='0' skill='73.000' nickname='DevilDaga10' clanName='clanforlife' class_id='0' online_id='~snip~@warface/GameClient' group_id='ad4fc6a9-4434-8858-e8277fbfab71' presence='17' experience='13782130' rank='73' banner_badge='4294967295' banner_mark='4294967295' banner_stripe='527' region_id='global'/></players><playersReserved/><team_colors><team_color id='1' color='4294907157'/><team_color id='2' color='4279655162'/></team_colors></core><session id='' status='0' game_progress='0' start_time='18446744011573954816' revision='2'/><custom_params friendly_fire='0' enemy_outlines='1' auto_team_balance='0' dead_can_chat='1' join_in_the_process='1' max_players='5' round_limit='0' class_restriction='253' inventory_slot='2113929215' revision='2'/><mission mission_key='767b0c7b-d0a2-4e66-8d04-3bdc2f01b3eb' no_teams='1' name='@na_mission_path01_1' setting='africa/africa_base' mode='pve' mode_name='@PvE_game_mode_desc' mode_icon='pve_icon' description='@mission_desc_africa_path' image='mapImgAfrica_training' difficulty='easy' type='trainingmission' time_of_day='9:06' revision='2'><objectives factor='1'><objective id='0' type='primary'/><objective id='14' type='secondary'/><objective id='17' type='secondary'/></objectives></mission><kick_vote_params success='0.6' timeout='60' cooldown='300' revision='1'/><room_master master='2367686' revision='2'/></game_room></invitation_request>
	*/

	/*
	<invitation_request from='DevilDaga10' ticket='15729104960274567555_2367686_DevilsBitch6' room_id='15729104960274567555' ms_resource='pvp_pro_5' is_follow='0' group_id='62a0634e-e13d-4dd4-8576-a13fb090e5f7'><game_room room_id='15729104960274567555' room_type='2'><core teams_switched='0' room_name='DevilDaga10&apos;s GAME ROOM' private='0' players='1' can_start='0' team_balanced='1' min_ready_players='4' revision='4'><players><player profile_id='2367686' team_id='1' status='0' observer='0' skill='0.000' nickname='DevilDaga10' clanName='clanforlife' class_id='3' online_id='22853416@warface/GameClient' group_id='62a0634e-e13d-4dd4-8576-a13fb090e5f7' presence='17' experience='13782130' rank='73' banner_badge='4294967295' banner_mark='4294967295' banner_stripe='527' region_id='global'/></players><playersReserved/><team_colors><team_color id='1' color='4294907157'/><team_color id='2' color='4279655162'/></team_colors></core><session id='' status='0' game_progress='0' start_time='18446744011573954816' revision='2'/><custom_params friendly_fire='0' enemy_outlines='0' auto_team_balance='1' dead_can_chat='1' join_in_the_process='1' max_players='16' round_limit='0' class_restriction='253' inventory_slot='2113929215' revision='2'/><mission mission_key='e4f56e59-97fb-4451-a137-7b44026d96b0' no_teams='0' name='@pvp_mission_display_name_tdm_airbase' setting='pvp/tdm_airbase' mode='tdm' mode_name='@pvp_tdm_game_mode_desc' mode_icon='pvp_tdm_game_mode_icon' description='@pvp_tdm_mission_desc' image='mapImgtdm_airbase' difficulty='normal' type='' time_of_day='12' revision='2'><objectives factor='1'/></mission><kick_vote_params success='0.6' timeout='60' cooldown='300' revision='1'/><room_master master='2367686' revision='2'/></game_room></invitation_request>
	*/

	char *server = get_info ( msg, "from='", "'", "Server" );
	char *data = wf_get_query_content ( msg );

	if ( !data )
		return;

	char *resource = get_info ( data, "ms_resource='", "'", NULL );
	char *ticket = get_info ( data, "ticket='", "'", NULL );
	char *room = get_info ( data, "room_id='", "'", NULL );
	char *nick_from = get_info ( data, " from='", "'", NULL );
	char *mission_data = get_info ( data, "<mission", ">", NULL );
	char *mission_type;
	if ( strstr ( mission_data, "mode='pve'" ) )
		mission_type = get_info ( mission_data, "type='", "'", NULL );
	else
		mission_type = get_info ( mission_data, "setting='pvp/", "'", NULL );

	LOGPRINT ( "%-20s" KGRN BOLD " %-20s " KWHT "@%s\n", "Invitation from", nick_from, mission_type );

	if ( server && resource && ticket && room )
	{
		int allowed = ( !session.commands.whitelist || strstr ( session.commands.whitelist, nick_from ) );

		/* 1. Confirm or refuse invitation */
		send_stream_format ( session.wfs,
							 "<iq to='%s' type='get'>"
							 " <query xmlns='urn:cryonline:k01'>"
							 "  <invitation_accept ticket='%s' result='%d'/>"
							 " </query>"
							 "</iq>",
							 server, ticket, ( session.gameroom.is_safemaster || !allowed ) );

		if ( !session.gameroom.is_safemaster )
		{
			if ( allowed )
				xmpp_iq_gameroom_join ( resource, room );	/* 2. Join the room */
			else
				LOGPRINT ( KRED BOLD "%-20s " KGRN "%s\n", "REJECTED", nick_from );
		}
	}

	free ( nick_from );
	free ( server );
	free ( ticket );
	free ( room );
	free ( resource );
	free ( mission_type );
	free ( mission_data );

	free ( data );
}

void xmpp_iq_invitation_request_r ( void )
{
	qh_register ( "invitation_request", 1, xmpp_iq_invitation_request_cb, NULL );
}