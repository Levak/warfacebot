/**
 * WarfaceBot, a blind XMPP client for Warface (FPS)
 * Copyright (C) 2015-2017 Levak Borok <levak92@gmail.com>
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

#include <string.h>

#include <wb_session.h>
#include <wb_xmpp_wf.h>

void cmd_switch(void)
{
    if (session.gameroom.curr_team == 1)
        session.gameroom.curr_team = 2;
    else
        session.gameroom.curr_team = 1;

    xmpp_iq_gameroom_setplayer(session.gameroom.curr_team,
                               GAMEROOM_READY,
                               session.profile.curr_class,
                               NULL, NULL);
}

void cmd_switch_wrapper(void)
{
    cmd_switch();
}
