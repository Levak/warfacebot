/**
 * WarfaceBot, a blind XMPP client for Warface (FPS)
 * Copyright (C) 2015 Levak Borok <levak92@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <wb_tools.h>
#include <wb_stream.h>
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>
#include <wb_session.h>

#include <stdlib.h>

static void xmpp_iq_gameroom_sync_cb(const char *msg_id,
                                     const char *msg,
                                     void *args)
{
    char *data = wf_get_query_content(msg);
    char *game_progress = get_info(data, "game_progress='", "'", NULL);

    /* If the room has started, leave ! */
    if (game_progress != NULL && strtoll(game_progress, 0, 10) > 0)
    {
        xmpp_iq_gameroom_leave();
    }

    free(game_progress);
    free(data);
}

void xmpp_iq_gameroom_sync_r(void)
{
    qh_register("gameroom_sync", xmpp_iq_gameroom_sync_cb, NULL);
}
