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
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>
#include <wb_session.h>

#include <stdlib.h>

static void xmpp_iq_gameroom_offer_cb(const char *msg_id,
                                      const char *msg,
                                      void *args)
{
    /* Answer:
       <iq from='masterserver@warface/pve_11' type='get'>
        <query xmlns='urn:cryonline:k01'>
         <data query_name='gameroom_offer' from='xxx' room_id='xxxx'
               token='xxxxxxxxxxxxxxx' ms_resource='pve_11'
               id='xxxxxxxxxxxxx' silent='1' compressedData='...'
               originalSize='1708'/>
        </query>
       </iq>
     */

    char *data = wf_get_query_content(msg);

    if (!data)
        return;

    char *resource = get_info(data, "ms_resource='", "'", NULL);
    char *room_id = get_info(data, "room_id='", "'", NULL);

    xmpp_iq_gameroom_join(resource, room_id);

    free(room_id);
    free(resource);
    free(data);
}

void xmpp_iq_gameroom_offer_r(void)
{
    qh_register("gameroom_offer", 1, xmpp_iq_gameroom_offer_cb, NULL);
}
