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

void xmpp_iq_broadcast_session_result_cb(const char *msg_id,
										  const char *msg,
										  void *args)
{
	/*
	<broadcast_session_result bcast_receivers="20511692@warface/GameClient,22853416@warface/GameClient,22910345@warface/GameClient,22911131@warface/GameClient">
  <player_result nickname="Tastan" experience="3900" pvp_rating_points="0" money="1800" gained_crown_money="0" no_crown_rewards="1" sponsor_points="0" bonus_experience="0" bonus_money="0" bonus_sponsor_points="0" experience_boost="0" money_boost="0" sponsor_points_boost="0" experience_boost_percent="0" money_boost_percent="0" sponsor_points_boost_percent="0" completed_stages="13" is_vip="0" score="44328" dynamic_multipliers_info="" dynamic_crown_multiplier="1">
	<profile_progression_update profile_id="1597755" mission_unlocked="none,trainingmission,easymission,normalmission,hardmission,survivalmission,zombieeasy,zombienormal,zombiehard,campaignsections,campaignsection1,campaignsection2,campaignsection3,volcanoeasy,volcanonormal,volcanohard,all" tutorial_unlocked="7" class_unlocked="29"/>
  </player_result>
  <player_result nickname="DevilDaga10" experience="11505" pvp_rating_points="0" money="5040" gained_crown_money="0" no_crown_rewards="1" sponsor_points="0" bonus_experience="0" bonus_money="0" bonus_sponsor_points="0" experience_boost="7605" money_boost="3240" sponsor_points_boost="0" experience_boost_percent="1.95" money_boost_percent="1.8" sponsor_points_boost_percent="1.45" completed_stages="13" is_vip="1" score="44328" dynamic_multipliers_info="" dynamic_crown_multiplier="1">
	<profile_progression_update profile_id="2367686" mission_unlocked="none,trainingmission,easymission,normalmission,hardmission,survivalmission,zombieeasy,zombienormal,zombiehard,campaignsections,campaignsection1,campaignsection2,campaignsection3,volcanoeasy,volcanonormal,volcanohard,all" tutorial_unlocked="7" tutorial_passed="7" class_unlocked="29"/>
  </player_result>
	*/

	puts(msg);

}

void xmpp_iq_broadcast_session_result_r(void)
{
	qh_register("broadcast_session_result", 1, xmpp_iq_broadcast_session_result_cb, NULL);
}