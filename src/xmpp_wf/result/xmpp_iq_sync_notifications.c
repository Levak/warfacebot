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

void xmpp_iq_sync_notifications_cb(const char *msg_id,
                                   const char *msg,
                                   void *args)
{
    /* Answer:
       <iq from='masterserver@warface/pve_11' type='get'>
        <query xmlns='urn:cryonline:k01'>
         <sync_notifications>
          <notif id='77644800' type='64' confirmation='1'
                 from_jid='masterserver@warface/pve_11'>
           <invitation initiator='xxxx' target='yyyy'/>
          </notif>
         </sync_notifications>
         </query>
       </iq>
    */

    char *data = wf_get_query_content(msg);

    if (data == NULL)
        return;

    const char *m = data;

    while ((m = strstr(m, "<notif")) != NULL)
    {
        char *notif = get_info(m, "<notif", "</notif>", NULL);

        xmpp_iq_confirm_notification(notif);

        free(notif);
        ++m;
    }

    free(data);
}

void xmpp_iq_sync_notifications_r(void)
{
    qh_register("sync_notifications", 1, xmpp_iq_sync_notifications_cb, NULL);
}
