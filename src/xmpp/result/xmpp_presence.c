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
#include <wb_stream.h>
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>
#include <wb_session.h>
#include <wb_dbus.h>

static void xmpp_presence_cb(const char *msg_id, const char *msg, void *args)
{
    char *jid = get_info(msg, "from='", "'", NULL);
    int is_joining = strstr(msg, "type='unavailable'") == NULL;

#ifdef DBUS_API
    dbus_api_emit_room_presence(jid, is_joining);
#endif /* DBUS_API */

    free(jid);
}

void xmpp_presence_r(void)
{
    qh_register("presence", 1, xmpp_presence_cb, NULL);
}
