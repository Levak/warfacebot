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
#include <wb_lang.h>

#include <wb_log.h>
#include <stdlib.h>

void xmpp_iq_shop_sync_consummables_cb(const char *msg_id,
                                       const char *msg,
                                       void *args)
{
    /* Answer:
       <iq from='masterserver@warface/xxx' type='get'>
        <query xmlns='urn:cryonline:k01'>
         <shop_sync_consumables session_id='xxxxxxxxxxxx'>
          <profile_items profile_id='xxxx'>
           <item id='xxxxxxxxx' name='coin_01' attached_to='0' config=''
                 slot='0' equipped='0' default='0' permanent='0'
                 expired_confirmed='0' buy_time_utc='1440538616'
                 quantity='24' added_quantity='2'/>
          </profile_items>
         </shop_sync_consumables>
        </query>
       </iq>
    */

    char *data = wf_get_query_content(msg);

    if (data == NULL)
        return;

    const char *m = data;

    while ((m = strstr(m, "<item")) != NULL)
    {
        char *item = get_info(m, "<item", "/>", NULL);
        char *item_name = get_info(item, "name='", "'", NULL);
        unsigned int quantity = get_info_int(item, " quantity='", "'", NULL);
        unsigned int added = get_info_int(item, "added_quantity='", "'", NULL);

        xprintf("%s: %d (+%d) %s",
                LANG(update_consummable),
                quantity,
                added,
                item_name);

        free(item_name);
        free(item);
        ++m;
    }

    free(data);
}

void xmpp_iq_shop_sync_consummables_r(void)
{
    qh_register("shop_sync_consumables", 1, xmpp_iq_shop_sync_consummables_cb, NULL);
}
