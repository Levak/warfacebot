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

#include <wb_xmpp_wf.h>
#include <wb_log.h>
#include <wb_cmd.h>

#include <stdio.h>

void parse_cmd(const char *buff_readline);

void cmd_exec(const char *file_name)
{
    if (file_name == NULL)
        return;

    FILE *f = fopen(file_name, "r");

    if (f == NULL)
    {
        xprintf("Cannot open file %s\n", file_name);

        return;
    }

    char *line = NULL;
    size_t len = 0;

    while (getline(&line, &len, f) != -1)
    {
        xprintf("%s", line);
        parse_cmd(line);
    }

    free(line);
    fclose(f);
}

void cmd_exec_wrapper(const char *file_name)
{
    cmd_exec(file_name);
}
