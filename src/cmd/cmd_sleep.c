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

#include <wb_threads.h>
#include <wb_cmd.h>

#include <stdlib.h>

void cmd_sleep(unsigned int delay)
{
    sleep(delay);
}

void cmd_sleep_wrapper(const char *delay_str)
{
    if (delay_str != NULL)
        cmd_sleep(strtol(delay_str, NULL, 10));
    else
        cmd_sleep(1);
}

int cmd_sleep_completions(struct list *l)
{
    return 1;
}
