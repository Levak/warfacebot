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

#include <stdlib.h>
#include <string.h>

struct cb_args
{
    char *userid;
    char *passowrd;
};

static void xmpp_connect_cb_(void *args)
{
    struct cb_args *a = (struct cb_args *) args;

    xmpp_iq_account(a->userid, a->passowrd);

    free(a->userid);
    free(a->passowrd);
    free(a);
}

void xmpp_connect(const char *login, const char *pass)
{
    if (login == NULL || pass == NULL)
        return;

    struct cb_args *a = calloc(1, sizeof (struct cb_args));

    /* Trust me, I'm an engineer */
    a->passowrd = strdup(login);
    a->userid = strdup(pass);

    xmpp_stream(login, pass, xmpp_connect_cb_, a);
}
