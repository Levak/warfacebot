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

#include <wb_tools.h>
#include <wb_stream.h>
#include <wb_session.h>
#include <wb_xml.h>
#include <wb_xmpp_wf.h>

#include <stdlib.h>
#include <stdio.h>

static void xmpp_iq_invitation_send_cb(const char *msg,
                                       enum xmpp_msg_type type,
                                       void *args)
{
    /* Answer :
       <iq to='masterserver@warface/pve_2' type='get'>
        <query xmlns='urn:cryonline:k01'>
         <invitation_send/>
        </query>
       </iq>
     */

    if (type & XMPP_TYPE_ERROR)
    {
        fprintf(stderr, msg);
    }
}

void xmpp_iq_invitation_send(const char *nickname, int is_follow,
                             f_query_callback cb, void *args)
{
    if (session.gameroom_jid != NULL)
    {
        char *nick = xml_serialize(nickname);

        t_uid id;

        idh_generate_unique_id(&id);
        idh_register(&id, 0, xmpp_iq_invitation_send_cb, NULL);
        qh_register("invitation_result", 0, cb, args);

        if (session.group_id == NULL)
            session.group_id = new_random_uuid();

        send_stream_format(session.wfs,
                           "<iq id='%s' type='get'"
                           "    to='masterserver@warface/%s'>"
                           " <query xmlns='urn:cryonline:k01'>"
                           "  <invitation_send nickname='%s' is_follow='%d'"
                           "                   group_id='%s'/>"
                           " </query>"
                           "</iq>",
                           &id, session.channel, nick, is_follow,
                           session.group_id);

        free(nick);
    }
}

