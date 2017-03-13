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
#include <wb_quickplay.h>
#include <wb_log.h>
#include <wb_lang.h>

#include <stdlib.h>

static void xmpp_iq_preinvite_cancel_cb(const char *msg_id,
                                        const char *msg,
                                        void *args)
{
    /* Answer;
       <iq from='xxxx@warface/GameClient' type='get'>
        <query xmlns='urn:cryonline:k01'>
         <preinvite_cancel uid='xxxxxxx' reason='xx'/>
        </query>
       </iq>
     */

    char *data = wf_get_query_content(msg);

    if (!data)
        return;

    char *jid = get_info(msg, "from='", "'", NULL);
    char *uid = get_info(data, "uid='", "'", NULL);
    int reason = get_info_int(data, "reason='", "'", NULL);

    if (jid != NULL
        && uid != NULL
        && session.quickplay.pre_uid != NULL
        && 0 == strcmp(uid, session.quickplay.pre_uid))
    {
        switch (reason)
        {
            case PREINVITE_EXPIRED:
            case PREINVITE_TIMEOUT:
                xprintf("%s", LANG(preinvite_expired));

                free(session.quickplay.pre_uid);
                session.quickplay.pre_uid = NULL;
                break;

            case PREINVITE_CANCELED_BY_MASTER:
                xprintf("%s", LANG(preinvite_canceled_by_master));

                free(session.quickplay.pre_uid);
                session.quickplay.pre_uid = NULL;
                break;

            case PREINVITE_CANCELED_BY_CLIENT:
                quickplay_preinvite_response(uid, jid, 0);
                break;
        }
    }

    free(uid);
    free(jid);
    free(data);
}

void xmpp_iq_preinvite_cancel_r(void)
{
    qh_register("preinvite_cancel", 1, xmpp_iq_preinvite_cancel_cb, NULL);
}
