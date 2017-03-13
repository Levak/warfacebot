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

void xmpp_iq_sponsor_info_updated_cb(const char *msg_id,
                                     const char *msg,
                                     void *args)
{
    /* Answer:
       <iq from='masterserver@warface/xxx' type='get'>
        <query xmlns='urn:cryonline:k01'>
         <sponsor_info_updated sponsor_id='0' sponsor_points='864'
             total_sponsor_points='2064' next_unlock_item='smg07_shop'>
          <unlocked_items>
           <item name='xxx' .../>
          </unlocked_items>
         </sponsor_info_updated>
        </query>
       </iq>
    */

    char *data = wf_get_query_content(msg);

    if (data == NULL)
        return;

    unsigned sponsor_id = get_info_int(data, "sponsor_id='", "'", NULL);
    unsigned points = get_info_int(data, "sponsor_points='", "'", NULL);
    unsigned total = get_info_int(data, "total_sponsor_points='", "'", NULL);
    char *next_item = get_info(data, "next_unlock_item='", "'", NULL);
    char *unlocked_items =
        get_info(data, "<unlocked_items>", "</unlocked_items>", NULL);

    if (unlocked_items != NULL)
    {
        const char *m = unlocked_items;
        while ((m = strstr(m, "<item")) != NULL)
        {
            char *item = get_info(m, "<item", "/>", NULL);
            char *item_name = get_info(item, "name='", "'", NULL);

            xprintf("%s: %s",
                    LANG(notif_unlock_item),
                    item_name);

            free(item_name);
            free(item);
            ++m;
        }
    }

    const char *sponsor = NULL;
    switch (sponsor_id)
    {
        case 0:
            sponsor = LANG(console_sponsor_weapon);
            break;
        case 1:
            sponsor = LANG(console_sponsor_outfit);
            break;
        case 2:
            sponsor = LANG(console_sponsor_equipment);
            break;
        default:
            break;
    }

    if (sponsor != NULL && sponsor[0])
    {
        xprintf("%s: %u (+%u) - %s %s",
                sponsor,
                total,
                points,
                LANG(notif_unlocking),
                next_item);
    }
    else
    {
        xprintf("%s: %u (+%u) - %s",
                sponsor,
                total,
                points,
                LANG(notif_unlocking_done));
    }

    free(unlocked_items);
    free(next_item);
    free(data);
}

void xmpp_iq_sponsor_info_updated_r(void)
{
    qh_register("sponsor_info_updated", 1, xmpp_iq_sponsor_info_updated_cb, NULL);
}
