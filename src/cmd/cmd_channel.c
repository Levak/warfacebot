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

#include <wb_xmpp_wf.h>
#include <wb_list.h>
#include <wb_cmd.h>

#include <string.h>

void cmd_channel(const char *channel)
{
    xmpp_iq_join_channel(channel, NULL, NULL);
}

void cmd_channel_wrapper(const char *channel)
{
    cmd_channel(channel);
}

int cmd_channel_completions(struct list *l)
{
    complete_channels(l);

    return 1;
}
