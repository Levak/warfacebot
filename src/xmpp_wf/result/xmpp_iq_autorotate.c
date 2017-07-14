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
#include <wb_masterserver.h>
#include <wb_log.h>

#include <stdlib.h>

static void _get_master_server_cb(const char *resource,
                                  int load_index,
                                  void *args)
{
    xmpp_iq_join_channel(resource, NULL, NULL);
}

static void _update_list(void *args)
{
    xmpp_iq_items(NULL, NULL);
    xmpp_iq_get_configs(NULL, NULL);

    xmpp_iq_get_master_server(cvar.online_pvp_rank,
                              cvar.online_channel_type,
                              _get_master_server_cb,
                              NULL);

}

static void xmpp_iq_autorotate_cb(const char *msg_id,
                                  const char *msg,
                                  void *args)
{
    /* Reply to peer to peer pings
       <iq from='k01.warface' type='get'>
        <query xmlns='urn:cryonline:k01'>
         <autorotate/>
        </query>
       </iq>
     */

    char *from = get_info(msg, "from='", "'", NULL);

    //if (from != NULL && 0 == strcmp(from, "k01.warface"))
    {
        xprintf("Auto-rotating...");
        masterserver_list_update(_update_list, NULL);
    }

    free(from);
}

void xmpp_iq_autorotate_r(void)
{
    qh_register("autorotate", 1, xmpp_iq_autorotate_cb, NULL);
}
