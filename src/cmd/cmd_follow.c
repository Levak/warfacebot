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

#include <wb_tools.h>
#include <wb_xmpp_wf.h>
#include <wb_log.h>
#include <wb_cmd.h>
#include <wb_lang.h>

static void cmd_follow_cb(const char *info, void *args)
{
    if (info == NULL)
    {
        eprintf("%s", LANG(error_no_user));
        return;
    }

    char *nickname = get_info(info, "nickname='", "'", NULL);
    char *online_id = get_info(info, "online_id='", "'", NULL);

    if (online_id != NULL && nickname != NULL)
        xmpp_iq_follow_send(nickname, online_id, NULL, NULL);

    free(online_id);
    free(nickname);
}

void cmd_follow(const char *nickname)
{
    if (nickname == NULL)
        return;

    xmpp_iq_profile_info_get_status(nickname, cmd_follow_cb, NULL);
}

void cmd_follow_wrapper(const char *nickname)
{
    cmd_follow(nickname);
}

int cmd_follow_completions(struct list *l)
{
    complete_buddies(l);

    return 1;
}
