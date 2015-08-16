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

#include <wb_session.h>
#include <wb_list.h>
#include <wb_friend.h>
#include <wb_clanmate.h>
#include <wb_mission.h>
#include <wb_room.h>

#include <stdlib.h>
#include <string.h>

struct session session = { 0 };

void session_init(int fd, const char *online_id)
{
    session.wfs = fd;
    session.active = 1;
    session.online_id = strdup(online_id);
    session.leaving = 0;
    session.ingameroom = 0;

    friend_list_init();
    clanmate_list_init();
    mission_list_init();
    room_list_init();
}

void session_free(void)
{
    friend_list_free();
    clanmate_list_free();
    mission_list_free();

    free(session.jid);
    free(session.nickname);
    free(session.active_token);
    free(session.profile_id);
    free(session.online_id);
    free(session.channel);
    free(session.clan_name);
}
