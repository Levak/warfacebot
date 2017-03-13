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
#include <wb_session.h>
#include <wb_xmpp.h>
#include <wb_quickplay.h>
#include <wb_log.h>

#include <stdlib.h>
#include <string.h>

static void xmpp_iq_gameroom_quickplay_started_cb(const char *msg_id,
                                const char *msg,
                                void *args)
{
    /* Answer:
       <iq from='k01.warface' type='get'>
        <query xmlns='urn:cryonline:k01'>
         <gameroom_quickplay_started
           uid='xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx' response_time='0'
           time_to_maps_reset_notification='40' timestamp='xxxxxxx'
           mission_hash='xxxx' content_hash='xxxx'/>
        </query>
       </iq>
     */

    char *data = wf_get_query_content(msg);

    if (data == NULL)
        return;

    char *uid = get_info(data, "uid='", "'", NULL);

    if (uid != NULL)
    {
        quickplay_started(uid);

        xmpp_send_iq_result(
            JID_K01,
            msg_id,
            "<query xmlns='urn:cryonline:k01'>"
            "<gameroom_quickplay_started uid='%s'/>"
            "</query>",
            uid);
    }

    free(uid);
    free(data);
}

void xmpp_iq_gameroom_quickplay_started_r(void)
{
    qh_register("gameroom_quickplay_started", 1,
                xmpp_iq_gameroom_quickplay_started_cb, NULL);
}
