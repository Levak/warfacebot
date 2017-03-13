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
#include <wb_quickplay.h>
#include <wb_log.h>

#include <stdlib.h>
#include <string.h>

static void xmpp_iq_preinvite_response_cb(const char *msg_id,
                                          const char *msg,
                                          void *args)
{
    /* Answer:
       <iq from='k01.warface' type='get'>
        <query xmlns='urn:cryonline:k01'>
         <preinvite_response uid='xxxxxxxxxxxxxxx'
            accepted='1' pid='xxxxxx' from='xxxxx'/>
        </query>
       </iq>
     */

    char *data = wf_get_query_content(msg);

    if (data == NULL)
        return;

    char *jid = get_info(msg, "from='", "'", NULL);
    char *from = get_info(data, "from='", "'", NULL);
    char *uid = get_info(data, "uid='", "'", NULL);
    char *pid = get_info(data, "pid='", "'", NULL);
    int accepted = get_info_int(data, "accepted='", "'", NULL);

    if (uid != NULL && jid != NULL && pid != NULL && from != NULL)
    {
        quickplay_preinvite_response(uid, jid, accepted);

        xmpp_send_iq_result(
            JID(jid),
            msg_id,
            "<query xmlns='urn:cryonline:k01'>"
            " <preinvite_response uid='%s' accepted='%d' from='%s' pid='%s'/>"
            "</query>",
            uid,
            accepted,
            from,
            pid);
    }

    free(from);
    free(jid);
    free(pid);
    free(uid);
    free(data);
}

void xmpp_iq_preinvite_response_r(void)
{
    qh_register("preinvite_response", 1,
                xmpp_iq_preinvite_response_cb, NULL);
}
