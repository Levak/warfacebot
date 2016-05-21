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

void xmpp_iq_broadcast_session_result_cb ( const char *msg_id,
										   const char *msg,
										   void *args )
{

	/*
	<iq from='k01.warface' to='22910345@warface/GameClient' type='get' id='1343823563'><query xmlns='urn:cryonline:k01'><data query_name='broadcast_session_result' compressedData='...' originalSize='4185'/></query></iq>
	*/

	/*
	<broadcast_session_result bcast_receivers='20511692@warface/GameClient,22910345@warface/GameClient,22061196@warface/GameClient,22911131@warface/GameClient,22911515@warface/GameClient'><player_result nickname='Tastan' experience='3900' pvp_rating_points='0' money='1800' gained_crown_money='0' no_crown_rewards='1' sponsor_points='0' bonus_experience='0' bonus_money='0' bonus_sponsor_points='0' experience_boost='0' money_boost='0' sponsor_points_boost='0' experience_boost_percent='0' money_boost_percent='0' sponsor_points_boost_percent='0' completed_stages='13' is_vip='0' score='44277' dynamic_multipliers_info='' dynamic_crown_multiplier='1'><profile_progression_update profile_id='1597755' mission_unlocked='none,trainingmission,easymission,normalmission,hardmission,survivalmission,zombieeasy,zombienormal,zombiehard,campaignsections,campaignsection1,campaignsection2,campaignsection3,volcanoeasy,volcanonormal,volcanohard,all' tutorial_unlocked='7' class_unlocked='29'/></player_result><player_result nickname='DevilsBitch6' experience='3900' pvp_rating_points='0' money='1800' gained_crown_money='0' no_crown_rewards='1' sponsor_points='0' bonus_experience='0' bonus_money='0' bonus_sponsor_points='0' experience_boost='0' money_boost='0' sponsor_points_boost='0' experience_boost_percent='0' money_boost_percent='0' sponsor_points_boost_percent='0' completed_stages='13' is_vip='0' score='44277' dynamic_multipliers_info='' dynamic_crown_multiplier='1'><profile_progression_update profile_id='2383428' mission_unlocked='none,trainingmission,easymission,normalmission,hardmission,survivalmission,zombieeasy,zombienormal,campaignsections,campaignsection1,campaignsection2,campaignsection3,volcanoeasy,volcanonormal,volcanohard,all' tutorial_unlocked='7' class_unlocked='5'/></player_result><player_result nickname='Nebel.' experience='3900' pvp_rating_points='0' money='2070' gained_crown_money='0' no_crown_rewards='1' sponsor_points='0' bonus_experience='0' bonus_money='0' bonus_sponsor_points='0' experience_boost='0' money_boost='270' sponsor_points_boost='0' experience_boost_percent='0' money_boost_percent='0.15' sponsor_points_boost_percent='0' completed_stages='13' is_vip='0' score='44277' dynamic_multipliers_info='' dynamic_crown_multiplier='1'><profile_progression_update profile_id='2108267' mission_unlocked='none,trainingmission,easymission,normalmission,hardmission,survivalmission,zombieeasy,zombienormal,zombiehard,campaignsections,campaignsection1,campaignsection2,campaignsection3,volcanoeasy,volcanonormal,volcanohard,all' tutorial_unlocked='7' tutorial_passed='7' class_unlocked='29'/></player_result><player_result nickname='DevilsBitch7' experience='3900' pvp_rating_points='0' money='1800' gained_crown_money='0' no_crown_rewards='1' sponsor_points='0' bonus_experience='0' bonus_money='0' bonus_sponsor_points='0' experience_boost='0' money_boost='0' sponsor_points_boost='0' experience_boost_percent='0' money_boost_percent='0' sponsor_points_boost_percent='0' completed_stages='13' is_vip='0' score='44277' dynamic_multipliers_info='' dynamic_crown_multiplier='1'><profile_progression_update profile_id='2383432' mission_unlocked='none,trainingmission,easymission,normalmission,hardmission,survivalmission,zombieeasy,zombienormal,campaignsections,campaignsection1,campaignsection2,campaignsection3,volcanoeasy,volcanonormal,volcanohard,all' tutorial_unlocked='7' class_unlocked='5'/></player_result><player_result nickname='DevilsBitch8' experience='3900' pvp_rating_points='0' money='1800' gained_crown_money='0' no_crown_rewards='1' sponsor_points='3000' bonus_experience='0' bonus_money='0' bonus_sponsor_points='0' experience_boost='0' money_boost='0' sponsor_points_boost='0' experience_boost_percent='0' money_boost_percent='0' sponsor_points_boost_percent='0' completed_stages='13' is_vip='0' score='44277' dynamic_multipliers_info='' dynamic_crown_multiplier='1'><profile_progression_update profile_id='2383439' mission_unlocked='none,trainingmission,easymission,normalmission,hardmission,zombieeasy,zombienormal,volcanoeasy,volcanonormal,volcanohard,all' tutorial_unlocked='7' class_unlocked='5'/></player_result></broadcast_session_result>
	*/

	char *data = wf_get_query_content ( msg );
	char *begin;
	FORMAT ( begin, "player_result nickname='%s'", session.profile.nickname );

	char *content = get_info ( data, begin, "</player_result>", NULL );

	int xp_gained = get_info_int ( content, "experience='", "'", NULL );
	int money_earned = get_info_int ( content, "money='", "'", NULL );
	int crowns_earned = get_info_int ( content, "gained_crown_money='", "'", NULL );
	int no_crowns = get_info_int ( content, "no_crown_rewards='", "'", NULL );
	int vp_gained = get_info_int ( content, "sponsor_points='", "'", NULL );
	int score = get_info_int ( content, "score='", "'", NULL );

	session.profile.experience += xp_gained;
	session.profile.money.game += money_earned;
	session.profile.money.crown += crowns_earned;

	char *rewards;
	FORMAT ( rewards, "SCORE = %-8d   XP = %-6d   MONEY = %-6d", score, xp_gained, money_earned );

	if ( vp_gained )
	{
		char *old_rewards = strdup ( rewards );
		FORMAT ( rewards, "%s   VP = %-6d", old_rewards, vp_gained );
		free ( old_rewards );
	}
	if ( !no_crowns )
	{
		char *old_rewards = strdup ( rewards );
		FORMAT ( rewards, "%s   CROWNS = %-6d", old_rewards, crowns_earned );
		free ( old_rewards );
	}

	LOGPRINT ( "%-20s " BOLD "%s\n", "GAME REWARDS", rewards );
	LOGPRINT ( "%-20s " BOLD "%d\n", "EXPERIENCE", session.profile.experience );
	LOGPRINT ( "%-20s " BOLD "%d\n", "MONEY", session.profile.money );

	free ( data );
	free ( content );
}

void xmpp_iq_broadcast_session_result_r ( void )
{
	qh_register ( "broadcast_session_result", 1, xmpp_iq_broadcast_session_result_cb, NULL );
}