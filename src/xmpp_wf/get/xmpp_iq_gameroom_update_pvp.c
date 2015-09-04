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
#include <wb_session.h>
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>

#include <stdio.h>
#include <stdlib.h>

void xmpp_iq_gameroom_update_pvp(const char *mission_key, enum pvp_mode flags,
                                 int max_players, int inventory_slot,
                                 f_id_callback cb, void *args)
{
    t_uid id;

    idh_generate_unique_id(&id);
    idh_register(&id, 0, cb, args);

    send_stream_format(session.wfs,
                       "<iq id='%s' to='masterserver@warface/%s' type='get'>"
                       " <query xmlns='urn:cryonline:k01'>"
                       "  <gameroom_update_pvp by_mission_key='1' mission_key='%s'"
                       "     private='%d'"
                       "     friendly_fire='%d'"
                       "     enemy_outlines='%d'"
                       "     auto_team_balance='%d'"
                       "     dead_can_chat='%d'"
                       "     join_in_the_process='%d'"
                       "     max_players='%d' inventory_slot='%d'>"
                       "   <class_rifleman enabled='1' class_id='0'/>"
                       "   <class_engineer enabled='1' class_id='4'/>"
                       "   <class_medic enabled='1' class_id='3'/>"
                       "   <class_sniper enabled='1' class_id='2'/>"
                       "  </gameroom_update_pvp>"
                       " </query>"
                       "</iq>",
                       &id, session.channel, mission_key,
                       flags & PVP_PRIVATE ? 1 : 0,
                       flags & PVP_FRIENDLY_FIRE ? 1 : 0,
                       flags & PVP_ENEMY_OUTLINES ? 1 : 0,
                       flags & PVP_AUTOBALANCE ? 1 : 0,
                       flags & PVP_DEADCHAT ? 1 : 0,
                       flags & PVP_ALLOWJOIN ? 1 : 0,
                       max_players, inventory_slot);
}

