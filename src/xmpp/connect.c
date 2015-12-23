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

#include <stdlib.h>
#include <stdio.h>

#ifdef USE_TLS
# include <openssl/err.h>
#endif /* USE_TLS */

void xmpp_connect(int fd, const char *login, const char *pass)
{
    /* TODO: Do handshake also in a query handler */

    /* Send Handshake */
    send_stream_ascii(fd, "<?xml version='1.0' ?>"
                    "<stream:stream to='warface'"
                    " xmlns='jabber:client'"
                    " xmlns:stream='http://etherx.jabber.org/streams'"
                    " xml:lang='en' version='1.0'>");
    flush_stream(fd);
    read_stream(fd); /* ~snip~ */
    read_stream(fd); /* <stream:stream> */
    read_stream(fd); /* <stream:features/>*/

#ifdef USE_TLS
    /* Enable TLS connection */
    send_stream_ascii(fd, "<starttls xmlns='urn:ietf:params:xml:ns:xmpp-tls'/>");
    flush_stream(fd);
    read_stream(fd); /* <proceed/> */

    if (init_tls_stream(fd) != 0)
        exit(1);

    send_stream_ascii(fd, "<?xml version='1.0' ?>"
                    "<stream:stream to='warface'"
                    " xmlns='jabber:client'"
                    " xmlns:stream='http://etherx.jabber.org/streams'"
                    " xml:lang='en' version='1.0'>");
    flush_stream(fd);
    read_stream(fd); /* <stream:stream> */
    read_stream(fd); /* <stream:features/>*/

#endif /* USE_TLS */

    /* SASL Authentification */
    char *logins_b64 = sasl_combine_logins(login, pass);
    send_stream_format(fd,
                       "<auth xmlns='urn:ietf:params:xml:ns:xmpp-sasl'"
                       " mechanism='WARFACE'>"
                       "%s"
                       "</auth>",
                       logins_b64);
    read_stream(fd);
    free(logins_b64);

    /* Open a new stream */
    send_stream_ascii(fd,
                    "<stream:stream to='warface'"
                    " xmlns='jabber:client'"
                    " xmlns:stream='http://etherx.jabber.org/streams'"
                    " xml:lang='en' version='1.0'>");
    flush_stream(fd);
    read_stream(fd);
    read_stream(fd);
}
