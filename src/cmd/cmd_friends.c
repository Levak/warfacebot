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

#include <wb_list.h>
#include <wb_session.h>
#include <wb_friend.h>
#include <wb_clanmate.h>
#include <wb_tools.h>

#include <stdio.h>

static void cmd_friend_online_cb_(struct friend* f, void *args)
{
    if (f->jid != NULL && !(f->status & (STATUS_AFK | STATUS_PLAYING)))
        LOGPRINT(KGRN BOLD "%s\n", f->nickname);
}

static void cmd_friend_afk_cb_(struct friend* f, void *args)
{
    if ((f->status & STATUS_AFK) && !(f->status & STATUS_PLAYING))
        LOGPRINT(KYEL BOLD "%s\n", f->nickname);
}

static void cmd_friend_ingame_cb_(struct friend* f, void *args)
{
    if (f->status & STATUS_PLAYING)
        LOGPRINT(KMAG BOLD "%s\n", f->nickname);
}

static void cmd_friend_offline_cb_(struct friend* f, void *args)
{
    if (f->jid == NULL)
        LOGPRINT(KCYN BOLD "%s\n", f->nickname);
}

static void cmd_clanmate_online_cb_(struct clanmate* f, void *args)
{
    if (f->jid != NULL && !(f->status & (STATUS_AFK | STATUS_PLAYING)))
		LOGPRINT(KGRN BOLD "%s\n", f->nickname);
}

static void cmd_clanmate_ingame_cb_(struct clanmate* f, void *args)
{
    if (f->status & STATUS_PLAYING)
        LOGPRINT(KMAG BOLD "%s\n", f->nickname);
}

static void cmd_clanmate_afk_cb_(struct clanmate* f, void *args)
{
    if ((f->status & STATUS_AFK) && !(f->status & STATUS_PLAYING))
		LOGPRINT(KYEL BOLD "%s\n", f->nickname);
}

static void cmd_clanmate_offline_cb_(struct clanmate* f, void *args)
{
    if (f->jid == NULL)
        LOGPRINT(KCYN BOLD "%s\n", f->nickname);
}

void cmd_friends(void)
{
    LOGPRINT("Friends " BOLD "(%u):\n", session.friends->length);

    list_foreach(session.friends,
                 (f_list_callback) cmd_friend_online_cb_, NULL);
	list_foreach(session.friends,
                 (f_list_callback) cmd_friend_ingame_cb_, NULL);
	list_foreach(session.friends,
                 (f_list_callback) cmd_friend_afk_cb_, NULL);
    list_foreach(session.friends,
                 (f_list_callback) cmd_friend_offline_cb_, NULL);

    LOGPRINT("Clan mates " BOLD "(%u):\n", session.clanmates->length);

    list_foreach(session.clanmates,
                 (f_list_callback) cmd_clanmate_online_cb_, NULL);
	list_foreach(session.clanmates,
                 (f_list_callback) cmd_clanmate_ingame_cb_, NULL);
    list_foreach(session.clanmates,
                 (f_list_callback) cmd_clanmate_afk_cb_, NULL);
    list_foreach(session.clanmates,
                 (f_list_callback) cmd_clanmate_offline_cb_, NULL);
}
