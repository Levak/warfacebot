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
#include <wb_session.h>
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>
#include <wb_game_version.h>

#include <stdlib.h>
#include <string.h>

static void xmpp_iq_create_profile_cb(const char *msg)
{
    char *data = wf_get_query_content(msg);

    if (xmpp_is_error(msg))
        return;

    session.profile_id = get_info(data, "profile_id='", "'", "PROFILE ID");
    session.nickname = get_info(data, "nick='", "'", "NICKNAME");

    free(data);
}

void xmpp_iq_create_profile(const char *channel)
{
    t_uid id;

    idh_generate_unique_id(&id);
    idh_register(&id, xmpp_iq_create_profile_cb, 0);

    session.channel = strdup(channel);

    send_stream_format(session.wfs,
                       "<iq id='%s' to='k01.warface' type='get'>"
                       "<query xmlns='urn:cryonline:k01'>"
                       "<create_profile version='" GAME_VERSION "'"
                       "                user_id='%s' token='%s'"
                       "                nickname='' resource='%s'/>"
                       "</query>"
                       "</iq>",
                       &id,
                       session.online_id, session.active_token,
                       session.channel);
}
