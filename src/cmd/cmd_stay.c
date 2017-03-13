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
#include <wb_log.h>
#include <wb_cmd.h>
#include <wb_lang.h>

#include <time.h>
#include <stdlib.h>
#include <string.h>

void cmd_stay(unsigned int secs)
{
    /* 2h max */
    if (secs > 2 * 60 * 60)
        secs = 2 * 60 * 60;

    {
        char *s = LANG_FMT(console_stay, secs);
        xprintf("%s", s);
        free(s);
    }

    session.gameroom.leave_timeout = time(NULL) + secs;
}

void cmd_stay_wrapper(const char *duration_str,
                      const char *unit)
{
    unsigned int duration = 1;
    unsigned int multiplier = 60 * 60;

    if (duration_str != NULL)
    {
        duration = strtol(duration_str, NULL, 10);
        multiplier = 1;
    }

    if (unit != NULL)
    {
        switch (unit[0])
        {
            case 'h':
                multiplier = 60 * 60;
                break;
            case 'm':
                multiplier = 60;
                break;
            case 's':
            default:
                multiplier = 1;
                break;
        }
    }

    cmd_stay(duration * multiplier);
}

int cmd_stay_completions(struct list *l, int arg_index)
{
    switch (arg_index)
    {
        case 1:
            break;

        case 2:
            list_add(l, strdup("sec"));
            list_add(l, strdup("min"));
            list_add(l, strdup("h"));
            break;

        default:
            break;
    }

    return 1;
}
