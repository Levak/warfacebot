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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void xmpp_iq_join_channel_cb(const char *msg)
{
    /* Answer
      <iq from='masterserver@warface/pve_12' to='xxxxxx@warface/GameClient' type='result'>
       <query xmlns='urn:cryonline:k01'>
        <data query_name='join_channel' compressedData='...' originalSize='13480'/>
       </query>
      </iq>
     */

    char *data = wf_get_query_content(msg);

    if (xmpp_is_error(msg))
    {
        fprintf(stderr, "Failed to join channel\n");
        return;
    }

    if (data != NULL)
    {
        char *exp = get_info(data, "experience='", "'", "EXPERIENCE");

        if (exp != NULL)
            session.experience = strtol(exp, NULL, 10);

        char *m = data;

        while ((m = strstr(m, "<notif")))
        {
            char *notif = get_info(m, "<notif", "</notif>", NULL);

            xmpp_iq_confirm_notification(notif);
            free(notif);
            ++m;
        }

        free(exp);
        free(data);
    }

    /* Inform to k01 our status */
    xmpp_iq_player_status(STATUS_ONLINE | STATUS_LOBBY);
}

void xmpp_iq_join_channel(const char *channel)
{
    int is_switch = session.channel != NULL;
    t_uid id;

    idh_generate_unique_id(&id);
    idh_register(&id, xmpp_iq_join_channel_cb, 0);

    if (channel != NULL)
    {
        free(session.channel);
        session.channel = strdup(channel);
    }

    /* Join CryOnline channel */
    send_stream_format(session.wfs,
                       "<iq id='%s' to='k01.warface' type='get'>"
                       "<query xmlns='urn:cryonline:k01'>"
                       "<%s_channel version='" GAME_VERSION "' token='%s'"
                       "     profile_id='%s' user_id='%s' resource='%s'"
                       "     user_data='' hw_id='' build_type='--release'/>"
                       "</query>"
                       "</iq>",
                       &id, is_switch ? "switch" : "join",
                       session.active_token, session.profile_id,
                       session.online_id, session.channel);
}
