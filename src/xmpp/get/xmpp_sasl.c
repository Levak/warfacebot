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
#include <wb_cvar.h>
#include <wb_log.h>
#include <wb_lang.h>

struct cb_args
{
    f_sasl_cb f;
    void *args;
};

static void xmpp_sasl_fail_cb_(const char *msg_id,
                               const char *msg,
                               void *args)
{
    if (msg == NULL)
        return;

    struct cb_args *a = (struct cb_args *) args;

    free(a);

    eprintf("%s", LANG(error_authentication));

    qh_remove("success");
    session.state = STATE_DEAD;
}

static void xmpp_sasl_cb_(const char *msg_id, const char *msg, void *args)
{
    /* Answer :
       <success xmlns="urn:ietf:params:xml:ns:xmpp-sasl"/>
     */

    if (msg == NULL)
        return;

    struct cb_args *a = (struct cb_args *) args;

    xmpp_send(
        "<stream:stream to='%s'"
        " xmlns='jabber:client'"
        " xmlns:stream='http://etherx.jabber.org/streams'"
        " xml:lang='en' version='1.0'>",
        session.online.jid.host);

    qh_remove("failure");

    xmpp_bind(cvar.online_resource, a->f, a->args);

    free(a);
}

void xmpp_sasl(const char *login, const char *password,
               f_sasl_cb cb, void *args)
{
    if (login == NULL || password == NULL)
        return;

    struct cb_args *a = calloc(1, sizeof(struct cb_args));

    a->f = cb;
    a->args = args;

    qh_register("success", 0, xmpp_sasl_cb_, a);
    qh_register("failure", 0, xmpp_sasl_fail_cb_, a);

    /* SASL Authentification */

    char *logins_b64 = sasl_combine_logins(login, password);

    xmpp_send(
        "<auth xmlns='urn:ietf:params:xml:ns:xmpp-sasl'"
        " mechanism='WARFACE'>"
        "%s"
        "</auth>",
        logins_b64);

    free(logins_b64);
}

