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
#include <wb_session.h>
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>

#include <stdlib.h>

struct cb_args
{
    f_get_last_seen_date_cb cb;
    void *args;
};

static void xmpp_iq_get_last_seen_date_cb(const char *msg,
                                          enum xmpp_msg_type type,
                                          void *args)
{
    /* Answer :
       <iq to='masterserver@warface/xxxx' type='get'>
        <query xmlns='urn:cryonline:k01'>
         <get_last_seen_date profile_id='xxx' last_seen='1467033431'/>
        </query>
       </iq>
     */

    struct cb_args *a = (struct cb_args *) args;

    if (type ^ XMPP_TYPE_ERROR)
    {
        char *data = wf_get_query_content(msg);

        if (data == NULL)
            return;

        char *profile_id = get_info(data, "profile_id='", "'", NULL);
        unsigned int last_seen =
            get_info_int(data, "last_seen='", "'", NULL);

        if (a->cb)
            a->cb(profile_id, last_seen, a->args);

        free(profile_id);
        free(data);
    }

    free(a);
}

void xmpp_iq_get_last_seen_date(const char *profile_id,
                                f_get_last_seen_date_cb cb,
                                void *args)
{
    if (profile_id == NULL)
        return;

    struct cb_args *a = calloc(1, sizeof (struct cb_args));

    a->cb = cb;
    a->args = args;

    xmpp_send_iq_get(
        JID_MS(session.online.channel),
        xmpp_iq_get_last_seen_date_cb, a,
        "<query xmlns='urn:cryonline:k01'>"
        " <get_last_seen_date profile_id='%s'/>"
        "</query>",
        profile_id);
}

