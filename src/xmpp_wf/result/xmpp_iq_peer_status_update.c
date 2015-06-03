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
#include <wb_friend.h>

#include <stdlib.h>
#include <string.h>

static void xmpp_iq_peer_status_update_cb(const char *msg_id,
                                          const char *msg,
                                          void *args)
{
    /* Answer
       <iq from='xxxxxxx@warface/GameClient' type='get'>
        <query xmlns='urn:cryonline:k01'>
         <peer_status_update nickname='xxxx' profile_id='xxxx' status='13'
                             experience='xxxx' place_token=''
                             place_info_token=''/>
        </query>
       </iq>
    */

    if (strstr(msg, "type='result'"))
        return;

    char *jid = get_info(msg, "from='", "'", NULL);
    char *nick = get_info(msg, "nickname='", "'", NULL);
    char *pid = get_info(msg, "profile_id='", "'", NULL);
    int status = get_info_int(msg, "status='", "'", NULL);
    int exp = get_info_int(msg, "experience='", "'", NULL);

    if (status <= STATUS_OFFLINE)
        friend_list_update(NULL, nick, pid, status, exp);
    else
        friend_list_update(jid, nick, pid, status, exp);

    free(jid);
    free(nick);
    free(pid);
}

void xmpp_iq_peer_status_update_r(void)
{
    qh_register("peer_status_update", 1, xmpp_iq_peer_status_update_cb, NULL);
}
