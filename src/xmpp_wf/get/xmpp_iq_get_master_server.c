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
#include <wb_log.h>
#include <wb_lang.h>

#include <stdlib.h>

struct cb_args
{
    f_get_master_server_cb cb;
    void *args;
};

static void xmpp_iq_get_master_server_cb(const char *msg,
                                         enum xmpp_msg_type type,
                                         void *args)
{
    /* Answer :
       <iq from='k01.warface' type='result'>
        <query xmlns='urn:cryonline:k01'>
         <get_master_server resource='pve_1' load_index='255'/>
        </query>
       </iq>
     */

    struct cb_args *a = (struct cb_args *) args;

    if (msg == NULL)
    {
        free(a);
        return;
    }

    if (type ^ XMPP_TYPE_ERROR)
    {

        char *resource = get_info(msg, "resource='", "'", LANG(channel));
        int load_index = get_info_int(msg, "load_index='", "'", NULL);

        if (resource != NULL)
        {
            if (a->cb != NULL)
                a->cb(resource, load_index, a->args);
        }

        free(resource);
    }
    else
        eprintf("%s", LANG(error_get_master_server));

    free(a);
}

void xmpp_iq_get_master_server(unsigned int rank,
                               const char *channel_type,
                               f_get_master_server_cb cb,
                               void *args)
{
    struct cb_args *a = calloc(1, sizeof (struct cb_args));

    a->cb = cb;
    a->args = args;

    if (channel_type == NULL)
        channel_type = "";

    xmpp_send_iq_get(
        JID_K01,
        xmpp_iq_get_master_server_cb, a,
        "<query xmlns='urn:cryonline:k01'>"
        "<get_master_server rank='%u' channel='%s'/>"
        "</query>",
        rank, channel_type);
}
