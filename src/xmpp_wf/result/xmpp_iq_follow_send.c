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

static void xmpp_iq_follow_send_cb(const char *msg_id,
                                   const char *msg,
                                   void *args)
{
    /* Answer:
       <iq from='xxxxx@warface/GameClient' id='uid000002c1' type='get'>
        <query xmlns='urn:cryonline:k01'>
         <follow_send nickname='xxxxxx' profile_id='xxxx'/>
        </query>
       </iq>
    */

    char *from_jid = get_info(msg, "from='", "'", NULL);
    char *nickname = get_info(msg, "nickname='", "'", NULL);

    /* Accept any follow request */
    send_stream_format(session.wfs,
                       "<iq to='masterserver@warface/%s' type='get'>"
                       " <query xmlns='urn:cryonline:k01'>"
                       "  <invitation_send nickname='%s' is_follow='1' group_id='%s'/>"
                       " </query>"
                       "</iq>",
                       session.channel, nickname, session.group_id);

    send_stream_format(session.wfs,
                       "<iq to='%s' id='%s' type='result'>"
                       " <query xmlns='urn:cryonline:k01'>"
                       "  <follow_send/>"
                       " </query>"
                       "</iq>",
                       from_jid, msg_id);

    free(nickname);
    free(from_jid);
}

void xmpp_iq_follow_send_r(void)
{
    qh_register("follow_send", xmpp_iq_follow_send_cb, NULL);
}
