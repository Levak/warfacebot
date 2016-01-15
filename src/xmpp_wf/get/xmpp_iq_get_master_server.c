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
#include <wb_stream.h>
#include <wb_session.h>
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>

#include <stdlib.h>
#include <string.h>

static void xmpp_iq_get_master_server_cb(const char *msg, void *args)
{
    /* Answer :
       <iq from='k01.warface' type='result'>
        <query xmlns='urn:cryonline:k01'>
         <get_master_server resource='pve_1'/>
        </query>
       </iq>
     */

    if (session.channel != NULL)
        free(session.channel);

    session.channel = get_info(msg, "resource='", "'", "RESOURCE");

    if (session.channel == NULL)
        session.channel = strdup("pve_12");

    xmpp_iq_get_account_profiles();
}

void xmpp_iq_get_master_server(const char *channel)
{
    t_uid id;

    idh_generate_unique_id(&id);
    idh_register(&id, 0, xmpp_iq_get_master_server_cb, NULL);

    if (channel == NULL)
        channel = "";

    if (strstr(channel, "pve"))
    {
        send_stream_format(session.wfs,
                           "<iq id='%s' to='k01.warface' type='get'>"
                           "<query xmlns='urn:cryonline:k01'>"
                           "<get_master_server channel='%s' search_type='%s'/>"
                           "</query>"
                           "</iq>",
                           &id, channel, "pve");
    }
    else
    {
        send_stream_format(session.wfs,
                           "<iq id='%s' to='k01.warface' type='get'>"
                           "<query xmlns='urn:cryonline:k01'>"
                           "<get_master_server channel='%s' rank='10'/>"
                           "</query>"
                           "</iq>",
                           &id, channel);
    }
}
