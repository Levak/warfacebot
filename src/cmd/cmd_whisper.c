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

#include <wb_log.h>
#include <wb_tools.h>
#include <wb_xmpp_wf.h>
#include <wb_cmd.h>
#include <wb_lang.h>

#include <string.h>

struct cb_args
{
    char *message;
};

static void cmd_whisper_cb(const char *info, void *args)
{
    struct cb_args *a = (struct cb_args *) args;

    if (info != NULL)
    {
        char *online_id = get_info(info, "online_id='", "'", NULL);
        char *nickname = get_info(info, "nickname='", "'", NULL);

        if (online_id != NULL && nickname != NULL)
            xmpp_send_message(nickname, online_id, a->message);

        free(nickname);
        free(online_id);
    }
    else
        eprintf("%s", LANG(error_no_user));

    free(a->message);
    free(a);
}

void cmd_whisper(const char *nickname, const char *message)
{
    if (nickname == NULL || message == NULL)
        return;

    struct cb_args *a = calloc(1, sizeof (struct cb_args));
    a->message = strdup(message);

    xmpp_iq_profile_info_get_status(nickname, cmd_whisper_cb, a);
}

void cmd_whisper_wrapper(const char *nickname,
                         const char *message)
{
    cmd_whisper(nickname, message);
}

int cmd_whisper_completions(struct list *l, int arg_index)
{
    switch (arg_index)
    {
        case 1:
            complete_gameroom_players(l);
            complete_buddies(l);
            break;

        case 2:
            break;

        default:
            break;
    }

    return 1;
}
