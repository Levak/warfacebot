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

#include <wb_tools.h>
#include <wb_session.h>
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>
#include <wb_masterserver.h>
#include <wb_cvar.h>
#include <wb_log.h>

static void _get_master_server_cb(const char *resource,
                                  int load_index,
                                  void *args)
{
    free(session.online.channel);
    session.online.channel = strdup(resource);

    struct masterserver *ms = masterserver_list_get(resource);

    free(session.online.channel_type);
    session.online.channel_type = NULL;

    if (ms != NULL)
    {
        session.online.channel_type = strdup(ms->channel);
    }

    xmpp_iq_get_account_profiles();
}

static void xmpp_iq_account_cb(const char *msg,
                               enum xmpp_msg_type type,
                               void *args)
{
    /* Answer :
       <iq from='k01.warface' to='XXXX@warface/GameClient' type='result'>
         <query xmlns='urn:cryonline:k01'>
           <account user='XXXX' active_token='$WF_XXXX_....'
                    survival_lb_enabled='1'>
             <masterservers>
               <server .../>
               ...
     */

    if (type & XMPP_TYPE_ERROR)
    {
        eprintf("Failed to log in\n");
        return;
    }

    free(session.online.active_token);
    free(session.online.id);

    session.online.status = STATUS_ONLINE;
    session.online.active_token =
        get_info(msg, "active_token='", "'", "ACTIVE TOKEN");
    session.online.id = get_info(msg, "user='", "'", NULL);

    const char *m = msg;

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

    masterserver_list_free();
    masterserver_list_init(masterservers);

    xmpp_iq_items(NULL, NULL);
    xmpp_iq_get_configs(NULL, NULL);

    xmpp_iq_get_master_server(cvar.online_pvp_rank,
                              cvar.online_channel_type,
                              _get_master_server_cb,
                              NULL);
}

void xmpp_iq_account(const char *login, const char *password)
{
    xmpp_send_iq_get(
        JID_K01,
        xmpp_iq_account_cb, NULL,
        "<query xmlns='urn:cryonline:k01'>"
        "<account login='%s' password='%s'/>"
        "</query>",
        login,
        password);
}
