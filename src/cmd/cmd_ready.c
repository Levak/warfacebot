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

#include <string.h>

#include <wb_session.h>
#include <wb_xmpp_wf.h>

void cmd_ready(const char *take_class)
{
    if (take_class != NULL)
    {
        if (strstr(take_class, "rif"))
            session.curr_class = CLASS_RIFLEMAN;
        else if (strstr(take_class, "med"))
            session.curr_class = CLASS_MEDIC;
        else if (strstr(take_class, "snip"))
            session.curr_class = CLASS_SNIPER;
        else if (strstr(take_class, "eng"))
            session.curr_class = CLASS_ENGINEER;
    }

    xmpp_iq_gameroom_setplayer(0, 1, session.curr_class, NULL, NULL);
}
