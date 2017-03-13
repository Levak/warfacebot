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

#include <string.h>

#include <wb_tools.h>
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>
#include <wb_session.h>
#include <wb_quickplay.h>
#include <wb_log.h>
#include <wb_status.h>
#include <wb_mission.h>
#include <wb_lang.h>

static void xmpp_iq_gameroom_on_expired_cb(const char *msg_id,
                                          const char *msg,
                                          void *args)
{
    /* Answer
       <iq from='masterserver@warface/pvp_pro_4' type='get'>
        <query xmlns='urn:cryonline:k01'>
         <gameroom_on_expired/>
        </query>
       </iq>
    */

    if (strstr(msg, "from='masterserver@warface") == NULL)
        return;

    char *data = wf_get_query_content(msg);

    if (data == NULL)
        return;

    xprintf("%s, %s",
            LANG(error_expired_missions),
            LANG(gameroom_leave));

    /* Refresh mission list */
    mission_list_update(NULL, NULL);

    status_set(STATUS_ONLINE | STATUS_LOBBY);
    xmpp_presence(session.gameroom.jid, XMPP_PRESENCE_LEAVE, NULL, NULL);

    free(session.gameroom.group_id);
    session.gameroom.group_id = NULL;
    free(session.gameroom.jid);
    session.gameroom.jid = NULL;

    gameroom_sync_free();
    quickplay_free();

    free(data);
}

void xmpp_iq_gameroom_on_expired_r(void)
{
    qh_register("gameroom_on_expired", 1, xmpp_iq_gameroom_on_expired_cb, NULL);
}
