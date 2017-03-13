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
#include <wb_threads.h>
#include <wb_xml.h>
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>

#include <stdlib.h>
#include <wb_log.h>
#include <unistd.h>

void xmpp_send_message(const char *to_login, const char *to_jid,
                       const char *msg)
{
    char *serialized = xml_serialize(msg);
    t_uid id;

    idh_generate_unique_id(&id);

    xprintf("\033[34;1m%s\033[0m: %s", session.profile.nickname, msg);

    sleep(rand() % 2 + 1); /* Take our time to answer */

    xmpp_send_iq_get(
        JID(to_jid),
        NULL, NULL,
        "<query xmlns='urn:cryonline:k01'>"
        "<message from='%s' nick='%s' message='%s'/>"
        "</query>",
        session.profile.nickname,
        to_login,
        serialized);

    free(serialized);
}

void xmpp_ack_message(const char *from_login, const char *from_jid,
                      const char *msg, const char *answer_id)
{
    char *serialized = xml_serialize(msg);

    xmpp_send_iq_result(
        JID(from_jid),
        answer_id,
        "<query xmlns='urn:cryonline:k01'>"
        "<message from='%s' nick='%s' message='%s'/>"
        "</query>",
        from_login,
        session.profile.nickname,
        serialized);

    free(serialized);
}
