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
#include <wb_session.h>

struct cb_args
{
    f_session_cb f;
    void *args;
};

static void xmpp_bind_cb(const char *msg,
                         enum xmpp_msg_type type,
                         void *args)
{
    /* Answer :
      <iq id='bind_1' type='result'>
         <bind xmlns='urn:ietf:params:xml:ns:xmpp-bind'>
           <jid>XXXX@warface.com/GameClient</jid>
         </bind>
       </iq>
    */

    struct cb_args *a = (struct cb_args *) args;

    if (type ^ XMPP_TYPE_ERROR)
    {
        free(session.xmpp.jid);

        session.xmpp.jid = get_info(msg, "<jid>", "</jid>", "JID");

        xmpp_iq_session(a->f, a->args);
    }

    free(a);
}

void xmpp_bind(const char *resource,
               f_bind_cb cb, void *args)
{
    struct cb_args *a = calloc(1, sizeof(struct cb_args));

    a->f = cb;
    a->args = args;

    /* Bind stream and get JID */
    xmpp_send_iq(
        NULL,
        XMPP_TYPE_SET,
        xmpp_bind_cb, a,
        "<bind xmlns='urn:ietf:params:xml:ns:xmpp-bind'>"
        "  <resource>%s</resource>"
        "</bind>",
        resource);
}
