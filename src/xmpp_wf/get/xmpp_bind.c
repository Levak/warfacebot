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

static void xmpp_bind_cb(const char *msg, void *args)
{
    /* Answer :
      <iq id='bind_1' type='result'>
         <bind xmlns='urn:ietf:params:xml:ns:xmpp-bind'>
           <jid>XXXX@warface.com/GameClient</jid>
         </bind>
       </iq>
    */

    if (xmpp_is_error(msg))
        return;

    free(session.jid);

    session.jid = get_info(msg, "<jid>", "</jid>", "JID");

    xmpp_iq_session();
}

void xmpp_bind(const char *resource)
{
    t_uid id;

    idh_generate_unique_id(&id);
    idh_register(&id, 0, xmpp_bind_cb, NULL);

    /* Bind stream and get JID */
    send_stream_format(session.wfs,
                       "<iq id='%s' type='set'>"
                       "  <bind xmlns='urn:ietf:params:xml:ns:xmpp-bind'>"
                       "    <resource>%s</resource>"
                       "  </bind>"
                       "</iq>",
                       &id, resource);
}
