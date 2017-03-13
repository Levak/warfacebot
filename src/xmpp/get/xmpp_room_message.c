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

#include <wb_xml.h>
#include <wb_session.h>
#include <wb_xmpp.h>

#include <stdlib.h>

void xmpp_room_message(const char *room_jid, const char *msg)
{
    if (room_jid == NULL)
        return;

    char *serialized = xml_serialize(msg);

    xmpp_send(
        "<message to='%s' type='groupchat'>"
        "<body>%s</body>"
        "</message>",
        room_jid, serialized);

    free(serialized);
}

