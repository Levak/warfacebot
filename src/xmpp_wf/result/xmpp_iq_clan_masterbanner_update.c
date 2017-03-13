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
#include <wb_session.h>
#include <wb_list.h>

#include <stdlib.h>
#include <wb_log.h>
#include <string.h>

static void xmpp_iq_clan_masterbanner_update_cb(const char *msg_id,
                                 const char *msg,
                                 void *args)
{
    /* Answer:
       <iq from='k01.warface' type='get'>
        <query xmlns='urn:cryonline:k01'>
         <clan_masterbanner_update master_badge='4294967295'
                                   master_stripe='20016'
                                   master_mark='7500'/>
        </query>
       </iq>
    */

    char *data = wf_get_query_content(msg);

    if (data == NULL)
        return;

    session.profile.clan.master.badge =
        get_info_int(data, "master_badge='", "'", NULL);
    session.profile.clan.master.stripe =
        get_info_int(data, "master_stripe='", "'", NULL);
    session.profile.clan.master.mark =
        get_info_int(data, "master_mark='", "'", NULL);

    free(data);
}

void xmpp_iq_clan_masterbanner_update_r(void)
{
    qh_register("clan_masterbanner_update", 1, xmpp_iq_clan_masterbanner_update_cb, NULL);
}
