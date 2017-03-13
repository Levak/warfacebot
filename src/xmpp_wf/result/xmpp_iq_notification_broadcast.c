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
#include <wb_xml.h>
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>

#include <wb_log.h>
#include <stdlib.h>

void xmpp_iq_notifications_broadcast_cb(const char *msg_id,
                                        const char *msg,
                                        void *args)
{
    /* Answer:
       <iq from='masterserver@warface/pve_11' type='get'>
        <query xmlns='urn:cryonline:k01'>
         <notification_broadcast bcast_receivers='x@x/GameClient,...' message=''>
          <notif id='0' type='512' confirmation='0'
                 from_jid='masterserver@warface/%s'>
           <announcement id='%u' is_system='1'
                         frequency='600' repeat_time='1'
                         message='%s' server='' channel='' place='1'/>
          </notif>
         </notification_broadcast>
        </query>
       </iq>
    */

    char *data = wf_get_query_content(msg);

    if (data == NULL)
        return;

    char *from = get_info(msg, "from='", "'", NULL);
    char *notif = get_info(data, "<notif", "</notif>", NULL);

    if (notif != NULL)
    {
        char *announcement = get_info(notif, "<announcement", "/>", NULL);

        if (announcement != NULL)
        {
            char *message = get_info(announcement, "message='", "'", NULL);

            xml_deserialize_inplace(&message);

            xprintf("%s -> \033[1;39m%s\033[0m", from, message);

            free(message);
        }

        free(announcement);
    }

    free(notif);
    free(data);
    free(from);
}

void xmpp_iq_notification_broadcast_r(void)
{
    qh_register("notification_broadcast", 1, xmpp_iq_notifications_broadcast_cb, NULL);
}
