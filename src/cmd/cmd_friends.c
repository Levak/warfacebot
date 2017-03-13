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

#include <wb_list.h>
#include <wb_session.h>
#include <wb_friend.h>
#include <wb_clanmate.h>
#include <wb_log.h>
#include <wb_lang.h>

static void cmd_friend_online_cb_(struct friend* f, void *args)
{
    if (f->jid != NULL)
        xprintf("\033[1;32m%20s\033[0m\t", f->nickname);
}

static void cmd_friend_offline_cb_(struct friend* f, void *args)
{
    if (f->jid == NULL)
        xprintf("\033[1;31m%20s\033[0m\t", f->nickname);
}

static void cmd_clanmate_online_cb_(struct clanmate* f, void *args)
{
    if (f->jid != NULL)
        xprintf("\033[1;32m%20s\033[0m\t", f->nickname);
}

static void cmd_clanmate_offline_cb_(struct clanmate* f, void *args)
{
    if (f->jid == NULL)
        xprintf("\033[1;31m%20s\033[0m\t", f->nickname);
}

void cmd_friends(void)
{
    xprintf("%s (%u):",
            LANG(console_friends),
            (unsigned) session.profile.friends->length);

    list_foreach(session.profile.friends,
                 (f_list_callback) cmd_friend_online_cb_, NULL);

    xprintf("\n");

    list_foreach(session.profile.friends,
                 (f_list_callback) cmd_friend_offline_cb_, NULL);

    xprintf("\n\n%s (%u):",
            LANG(console_clanmates),
            (unsigned) session.profile.clanmates->length);

    list_foreach(session.profile.clanmates,
                 (f_list_callback) cmd_clanmate_online_cb_, NULL);

    xprintf("\n");

    list_foreach(session.profile.clanmates,
                 (f_list_callback) cmd_clanmate_offline_cb_, NULL);

    xprintf("\n");
}

void cmd_friends_wrapper(void)
{
    cmd_friends();
}
