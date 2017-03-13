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
#include <wb_masterserver.h>

#include <stdlib.h>
#include <string.h>

struct cb_args
{
    f_get_master_servers_cb cb;
    void *args;
};

static void xmpp_iq_get_master_servers_cb(const char *msg,
                                         enum xmpp_msg_type type,
                                         void *args)
{
    /* Answer :
       <iq from='k01.warface' type='result'>
        <query xmlns='urn:cryonline:k01'>
         <get_master_servers>
          <masterservers>
           <server resource='pvp_pro_2' server_id='302'
                   channel='pvp_pro' rank_group='all' load='0.590588'
                   online='501' min_rank='13' max_rank='80' bootstrap=''>
            <load_stats>
             <load_stat type='quick_play' value='251'/>
             <load_stat type='survival' value='255'/>
             <load_stat type='pve' value='255'/>
            </load_stats>
           </server>
          ...
        </query>
       </iq>
     */

    struct cb_args *a = (struct cb_args *) args;

    if (type & XMPP_TYPE_ERROR)
    {
        free(a);
        return;
    }

    char *data = wf_get_query_content(msg);

    if (data == NULL)
    {
        free(a);
        return;
    }

    const char *m = data;

    struct list *masterservers = masterserver_list_new();

    while ((m = strstr(m, "<server ")))
    {
        char *server = get_info(m, "<server ", "/>", NULL);

        struct masterserver *ms =
            calloc(1, sizeof (struct masterserver));

        ms->server_id = get_info_int(m, "server_id='", "'", NULL);
        ms->online = get_info_int(m, "online='", "'", NULL);
        ms->min_rank = get_info_int(m, "min_rank='", "'", NULL);
        ms->max_rank = get_info_int(m, "max_rank='", "'", NULL);
        ms->resource = get_info(m, "resource='", "'", NULL);
        ms->channel = get_info(m, "channel='", "'", NULL);
        ms->rank_group = get_info(m, "rank_group='", "'", NULL);
        ms->bootstrap = get_info(m, "bootstrap='", "'", NULL);
        ms->load = get_info_float(m, "load='", "'", NULL);

        list_add(masterservers, ms);

        free(server);
        ++m;
    }

    if (a->cb)
        a->cb(masterservers, a->args);
    else
        list_free(masterservers);

    free(data);
    free(a);
}

void xmpp_iq_get_master_servers(f_get_master_servers_cb cb, void *args)
{
    struct cb_args *a = calloc(1, sizeof (struct cb_args));

    a->cb = cb;
    a->args = args;

    xmpp_send_iq_get(
        JID_K01,
        xmpp_iq_get_master_servers_cb, a,
        "<query xmlns='urn:cryonline:k01'>"
        "<get_master_servers/>"
        "</query>",
        NULL);
}
