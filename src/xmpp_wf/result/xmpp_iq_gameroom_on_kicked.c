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

#include <string.h>

#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>
#include <wb_session.h>

static void xmpp_iq_gameroom_on_kicked_cb(const char *msg_id,
                                          const char *msg,
                                          void *args)
{
    /* Answer
       <iq from='masterserver@warface/pvp_pro_4' type='get'>
        <query xmlns='urn:cryonline:k01'>
         <gameroom_on_kicked reason='1'/>
        </query>
       </iq>
    */

    if (strstr(msg, "from='masterserver@warface") == NULL)
        return;

    session.ingameroom = 0;

    xmpp_iq_player_status(STATUS_ONLINE | STATUS_LOBBY);
    xmpp_presence(session.gameroom_jid, 1);
    free(session.gameroom_jid);
    session.gameroom_jid = NULL;
}

void xmpp_iq_gameroom_on_kicked_r(void)
{
    qh_register("gameroom_on_kicked", 1, xmpp_iq_gameroom_on_kicked_cb, NULL);
}
