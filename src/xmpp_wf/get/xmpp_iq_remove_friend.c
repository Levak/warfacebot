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
#include <wb_friend.h>
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>
#include <wb_lang.h>

#include <stdlib.h>

static void xmpp_iq_remove_friend_cb(const char *msg,
                                     enum xmpp_msg_type type,
                                     void *args)
{
    /* Answer :
       <iq to='masterserver@warface/pve_2' type='result'>
        <query xmlns='urn:cryonline:k01'>
         <remove_friend target='xxxxxx'/>
        </query>
       </iq>
     */

    if (type & XMPP_TYPE_ERROR)
        return;

    char *nickname = get_info(msg, "target='", "'",
                              LANG(console_removed_friend));

    friend_list_remove(nickname);

    free(nickname);
}

void xmpp_iq_remove_friend(const char *nickname)
{
    xmpp_send_iq_get(
        JID_MS(session.online.channel),
        xmpp_iq_remove_friend_cb, NULL,
        "<query xmlns='urn:cryonline:k01'>"
        " <remove_friend target='%s'/>"
        "</query>",
        nickname);
}

