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
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>

#include <stdlib.h>
#include <wb_log.h>
#include <time.h>

static void xmpp_print_number_of_occupants_cb(const char *msg,
                                              enum xmpp_msg_type type,
                                              void *args)
{
    unsigned int num = get_info_int(
        msg,
        "var='muc#roominfo_occupants'><value>",
        "</value>",
        NULL);

    eprintf("%u %u", (unsigned) time(NULL), num);
}

void xmpp_print_number_of_occupants(int wfs, const char *room)
{
    xmpp_send_iq_get(
        JID(room),
        xmpp_print_number_of_occupants_cb, NULL,
        "<query xmlns='http://jabber.org/protocol/disco#info'/>",
        NULL);
}
