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

#include <string.h>
#include <stdio.h>

#include <wb_tools.h>
#include <wb_stream.h>
#include <wb_session.h>
#include <wb_xml.h>
#include <wb_xmpp_wf.h>

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
        fprintf(stderr, "No such connected user\n");

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
