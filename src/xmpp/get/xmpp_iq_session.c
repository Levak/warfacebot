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

#include <wb_stream.h>
#include <wb_session.h>
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>

struct cb_args
{
    f_session_cb f;
    void *args;
};

void xmpp_iq_session_cb(const char *msg,
                        enum xmpp_msg_type type,
                        void *args)
{
    struct cb_args *a = (struct cb_args *) args;

    if (a->f != NULL)
        a->f(a->args);

    free(a);
}


void xmpp_iq_session(f_session_cb cb, void *args)
{
    struct cb_args *a = calloc(1, sizeof(struct cb_args));

    a->f = cb;
    a->args = args;

    /* Bind the session */
    xmpp_send_iq_set(
        NULL,
        xmpp_iq_session_cb, a,
        "<session xmlns='urn:ietf:params:xml:ns:xmpp-session'/>",
        NULL);
}
