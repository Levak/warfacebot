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

#include <wb_session.h>
#include <wb_xmpp_wf.h>
#include <wb_cmd.h>

#include <string.h>

void cmd_ready(const char *take_class)
{
    enum class class = session.profile.curr_class;

    if (take_class != NULL)
    {
        if (strstr(take_class, "rif"))
            class = CLASS_RIFLEMAN;
        else if (strstr(take_class, "med"))
            class = CLASS_MEDIC;
        else if (strstr(take_class, "snip"))
            class = CLASS_SNIPER;
        else if (strstr(take_class, "eng"))
            class = CLASS_ENGINEER;
        else if (strstr(take_class, "hea"))
            class = CLASS_HEAVY;
    }

    status_update_class(class);

    xmpp_iq_gameroom_setplayer(session.gameroom.curr_team,
                               GAMEROOM_READY,
                               class,
                               NULL, NULL);
}

void cmd_ready_wrapper(const char *take_class)
{
    cmd_ready(take_class);
}

int cmd_ready_completions(struct list *l)
{
    list_add(l, strdup("rifleman"));
    list_add(l, strdup("medic"));
    list_add(l, strdup("engineer"));
    list_add(l, strdup("sniper"));
    list_add(l, strdup("heavy"));

    return 1;
}
