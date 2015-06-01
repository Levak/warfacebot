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
#include <wb_session.h>
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>

static void xmpp_iq_account_cb(const char *msg, void *args)
{
    /* Answer :
       <iq from='k01.warface' to='XXXX@warface/GameClient' type='result'>
         <query xmlns='urn:cryonline:k01'>
           <account user='XXXX' active_token='$WF_XXXX_....'
            load_balancing_type='server'>
             <masterservers>
               <server .../>
               ...
     */

    if (xmpp_is_error(msg))
        return;

    free(session.active_token);

    session.status = STATUS_ONLINE;
    session.active_token = get_info(msg, "active_token='", "'", "ACTIVE TOKEN");

    xmpp_iq_get_master_server("pve");
}

void xmpp_iq_account(void)
{
    t_uid id;

    idh_generate_unique_id(&id);
    idh_register(&id, 0, xmpp_iq_account_cb, NULL);

    send_stream_format(session.wfs,
                       "<iq id='%s' to='k01.warface' type='get'>"
                       "<query xmlns='urn:cryonline:k01'>"
                       "<account login='%s'/>" /* Don't put any space there ! */
                       "</query>"
                       "</iq>",
                       &id, session.online_id);
}
