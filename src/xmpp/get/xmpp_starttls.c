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

#include <wb_stream.h>
#include <wb_session.h>
#include <wb_xmpp.h>

#include <string.h>
#include <stdio.h>
#include <unistd.h>

struct cb_args
{
    char *login;
    char *password;
    f_stream_cb f;
    void *args;
};

#ifdef USE_TLS
static void xmpp_starttls_cb_(const char *msg_id, const char *msg, void *args)
{
    /* Answer :
       <proceed xmlns="urn:ietf:params:xml:ns:xmpp-tls"/>
     */

    struct cb_args *a = (struct cb_args *) args;

    if (init_tls_stream(session.wfs) != 0)
		session.active = 0;
	else
		xmpp_stream(a->login, a->password, a->f, a->args);

    free(a->login);
    free(a->password);
    free(a);
}

void xmpp_starttls(const char *login, const char *password,
                 f_stream_cb cb, void *args)
{
    if (login == NULL || password == NULL)
        return;

    struct cb_args *a = calloc(1, sizeof(struct cb_args));

    a->login = strdup(login);
    a->password = strdup(password);
    a->f = cb;
    a->args = args;

    qh_register("proceed", 0, xmpp_starttls_cb_, a);

    /* Send Handshake */
    send_stream_format(session.wfs,
                       "<starttls xmlns='urn:ietf:params:xml:ns:xmpp-tls'/>");
}
#else /* USE_TLS */
void xmpp_starttls(const char *login, const char *password,
                 f_stream_cb cb, void *args)
{
    xmpp_stream(login, password, cb, args);
}
#endif /* !USE_TLS */

