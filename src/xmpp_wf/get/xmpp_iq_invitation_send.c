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
#include <wb_xml.h>
#include <wb_xmpp_wf.h>

#include <stdlib.h>
#include <wb_log.h>

static void xmpp_iq_invitation_send_cb(const char *msg,
                                       enum xmpp_msg_type type,
                                       void *args)
{
    /* Answer :
       <iq to='masterserver@warface/pve_2' type='result'>
        <query xmlns='urn:cryonline:k01'>
         <invitation_send/>
        </query>
       </iq>
     */

    if (msg == NULL)
        return;

    int result = INVIT_PENDING;
    char *data = wf_get_query_content(msg);

    if (!data)
        return;

    if (type & XMPP_TYPE_ERROR)
    {
        char *channel = get_info(msg, "from='masterserver@warface/", "'", NULL);
        char *nickname = get_info(data, "nickname='", "'", NULL);
        int custom_code = get_info_int(msg, "custom_code='", "'", NULL);

        if (custom_code != 0)
            result = custom_code;

        invitation_complete(nickname, channel, result, 0);

        free(nickname);
        free(channel);
    }

    free(data);
}

void xmpp_iq_invitation_send(const char *nickname, int is_follow,
                             f_invitation_result_cb cb, void *args)
{
    if (session.gameroom.jid != NULL)
    {
        char *nick = xml_serialize(nickname);

        invitation_register(nickname, 0, cb, args);

        if (session.gameroom.group_id == NULL)
            session.gameroom.group_id = new_random_uuid();

        xmpp_send_iq_get(
            JID_MS(session.online.channel),
            xmpp_iq_invitation_send_cb, NULL,
            " <query xmlns='urn:cryonline:k01'>"
            "  <invitation_send nickname='%s' is_follow='%d'"
            "                   group_id='%s'/>"
            " </query>",
            nick,
            is_follow,
            session.gameroom.group_id);

        free(nick);
    }
}

