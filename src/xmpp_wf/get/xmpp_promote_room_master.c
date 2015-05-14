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

static void xmpp_promote_room_master_cb(const char *msg)
{
    /* Answer
       <iq from='k01.warface' to='19997306@warface/GameClient' type='result'>
        <query xmlns='urn:cryonline:k01'>
         <profile_info_get_status nickname='xxxxxx'>
          <profile_info>
           <info nickname='xxxxx' online_id='xxxxx@warface/GameClient'
                 status='13' profile_id='xxx' user_id='xxxxxx' rank='xx'
                 tags='' login_time='xxxxxxxxxxx'/>
          </profile_info>
         </profile_info_get_status>
        </query>
       </iq>
     */

    char *profile_id = get_info(msg, "profile_id='", "'", "PROFILE ID");
    send_stream_format(session.wfs,
                       "<iq to='masterserver@warface/%s' type='get'>"
                       " <query xmlns='urn:cryonline:k01'>"
                       "  <gameroom_promote_to_host new_host_profile_id='%s'/>"
                       " </query>"
                       "</iq>",
                       session.channel, profile_id);

}

void xmpp_promote_room_master(const char *nickname)
{
    t_uid id;

    idh_generate_unique_id(&id);
    idh_register(&id, xmpp_promote_room_master_cb, 0);

    /* Ask server the account details of someone */
    send_stream_format(session.wfs,
                       "<iq to='k01.warface' id='%s' type='get'>"
                       "<query xmlns='urn:cryonline:k01'>"
                       "<profile_info_get_status nickname='%s'/>"
                       "</query>"
                       "</iq>",
                       &id, nickname);
}

