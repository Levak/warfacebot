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

#include <wb_session.h>
#include <wb_list.h>
#include <wb_friend.h>
#include <wb_clanmate.h>
#include <wb_mission.h>
#include <wb_room.h>
#include <wb_xmpp_wf.h>

#include <stdlib.h>
#include <string.h>
#include <time.h>

struct session session = { 0 };

void session_init(int fd)
{
    time_t now = time(NULL);

    memset(&session, 0, sizeof (struct session));

    session.wfs = fd;
    session.active = 1;

    session.xmpp.last_query = now;

    session.online.last_status_update = now;

    session.gameroom.leave_timeout = now;

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
    gameroom_sync_free();
    room_list_free();

    if (session.wf.shop.offers != NULL)
        list_free(session.wf.shop.offers);

    free(session.xmpp.jid);

    free(session.online.id);
    free(session.online.channel);
    free(session.online.active_token);

    free(session.online.place_token);
    free(session.online.place_info_token);
    free(session.online.mode_info_token);
    free(session.online.mission_info_token);

    free(session.gameroom.jid);
    free(session.gameroom.group_id);

    free(session.profile.id);
    free(session.profile.nickname);
    free(session.profile.clan.name);
    free(session.profile.primary_weapon);

    memset(&session, 0, sizeof (struct session));
}
