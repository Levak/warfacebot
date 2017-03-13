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
#include <wb_xmpp_wf.h>
#include <wb_cvar.h>
#include <wb_status.h>

#include <time.h>

static void xmpp_iq_gameroom_leave_cb(const char *msg,
                                      enum xmpp_msg_type type,
                                      void *args)
{
    /* Answer :
       <iq to='masterserver@warface/pve_2' type='get'>
        <query xmlns='urn:cryonline:k01'>
         <gameroom_leave/>
        </query>
       </iq>
     */

    session.gameroom.leaving = 0;
    session.gameroom.leave_timeout = time(NULL);

    if (type & XMPP_TYPE_ERROR)
        return;

    if (cvar.wb_safemaster)
    {
        cvar.wb_safemaster = 0;

        /* Cheaty way of deregistering query handler */
        qh_handle("presence", NULL, NULL);
    }

    xmpp_presence(session.gameroom.jid, XMPP_PRESENCE_LEAVE, NULL, NULL);

    free(session.gameroom.group_id);
    session.gameroom.group_id = NULL;
    free(session.gameroom.jid);
    session.gameroom.jid = NULL;

    gameroom_sync_free();

    status_set(STATUS_ONLINE | STATUS_LOBBY);
}

void xmpp_iq_gameroom_leave(void)
{
    if (session.gameroom.leaving || session.gameroom.jid == NULL)
        return;

    session.gameroom.leaving = 1;

    /* Leave the game room */
    xmpp_send_iq_get(
        JID_MS(session.online.channel),
        xmpp_iq_gameroom_leave_cb, NULL,
        "<query xmlns='urn:cryonline:k01'>"
        " <gameroom_leave/>"
        "</query>",
        NULL);
}

