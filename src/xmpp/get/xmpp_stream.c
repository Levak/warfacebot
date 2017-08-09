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
#include <wb_tools.h>
#include <wb_cvar.h>
#include <wb_log.h>

#include <string.h>

struct cb_args
{
    char *login;
    char *password;
    f_stream_cb f;
    void *args;
};

static void xmpp_error_cb_(const char *msg_id, const char *msg, void *args)
{
    if (msg == NULL)
        return;

    char *error = get_info(msg, "><", " ", NULL);
    char *error_text = get_info(msg, "<text", "</text>", NULL);
    const char *reason = error_text;

    if (error_text != NULL)
    {
        const char *p = strstr(error_text, ">");

        if (p != NULL)
            reason = p + 1;
    }

    if (reason != NULL)
        eprintf("Stream error (reason: %s '%s')", error, reason);
    else
        eprintf("Stream error (reason: %s)", error);

    free(error_text);
    free(error);

    session.state = STATE_DEAD;
}

static void xmpp_features_cb_(const char *msg_id, const char *msg, void *args)
{
    /* Answer :
       <stream:features>
        <starttls xmlns='urn:ietf:params:xml:ns:xmpp-tls'>
         <required/>
        </starttls>
        <mechanisms xmlns='urn:ietf:params:xml:ns:xmpp-sasl'>
         <mechanism>WARFACE</mechanism>
        </mechanisms>
       </stream:features>
     */

    if (msg == NULL)
        return;

    struct cb_args *a = (struct cb_args *) args;

#ifdef USE_TLS
    if (cvar.online_use_tls
        && strstr(msg, "<starttls") != NULL)
    {
        xmpp_starttls(a->login, a->password, a->f, a->args);
    }
    else
#endif /* USE_TLS */
    {
        xmpp_sasl(a->login, a->password, a->f, a->args);
    }

    free(a->login);
    free(a->password);
    free(a);
}

static void xmpp_stream_cb_(const char *msg_id, const char *msg, void *args)
{
    /* Answer :
       <?xml version='1.0'?>
       <stream:stream xmlns='jabber:client'
                      xmlns:stream='http://etherx.jabber.org/streams'
                      id='4235063168' from='warface' version='1.0' xml:lang='en'>
    */

    if (msg == NULL)
        return;

    if (strstr(msg, "stream:features") != NULL)
        xmpp_features_cb_(msg_id, msg, args);
}

void xmpp_stream(const char *login, const char *password,
                 f_stream_cb cb, void *args)
{
    if (login == NULL || password == NULL)
        return;

    struct cb_args *a = calloc(1, sizeof(struct cb_args));

    a->login = strdup(login);
    a->password = strdup(password);
    a->f = cb;
    a->args = args;

    qh_register("stream:stream", 0, xmpp_stream_cb_, (void *) a);
    qh_register("?xml", 0, xmpp_stream_cb_, (void *) a);
    qh_register("stream:features", 0, xmpp_features_cb_, (void *) a);
    qh_register("stream:error", 1, xmpp_error_cb_, NULL);

    /* Open stream */
    xmpp_send(
        "<?xml version='1.0' ?>"
        "<stream:stream to='%s'"
        " xmlns='jabber:client'"
        " xmlns:stream='http://etherx.jabber.org/streams'"
        " xml:lang='en' version='1.0'>",
        session.online.jid.host);
}

