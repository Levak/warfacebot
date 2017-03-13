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
#include <wb_xmpp_wf.h>
#include <wb_log.h>
#include <wb_status.h>
#include <wb_lang.h>

#include <stdlib.h>

struct cb_args
{
    f_gameroom_setinfo_cb cb;
    void *args;
};

static void xmpp_iq_gameroom_setinfo_cb(const char *msg,
                                        enum xmpp_msg_type type,
                                        void *args)
{
    /* Answer :
       <iq to='masterserver@warface/pve_2' type='get'>
        <query xmlns='urn:cryonline:k01'>
         <data query_name='gameroom_setinfo' compressedData='...'
               originalSize='42'/>
        </query>
       </iq>
     */

    struct cb_args *a = (struct cb_args *) args;

    if (type & XMPP_TYPE_ERROR)
    {
        const char *reason = NULL;

        int code = get_info_int(msg, "code='", "'", NULL);
        int custom_code = get_info_int(msg, "custom_code='", "'", NULL);

        switch (code)
        {
            case 8:
                switch (custom_code)
                {
                    case 1:
                        reason = LANG(error_unknown_mission);
                        break;
                    case 7:
                        reason = LANG(error_room_started);
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }

        if (reason != NULL)
            eprintf("%s (%s)",
                    LANG(error_gameroom_setinfo),
                    reason);
        else
            eprintf("%s (%i:%i)",
                    LANG(error_gameroom_setinfo),
                    code,
                    custom_code);
    }
    else
    {
        char *data = wf_get_query_content(msg);

        if (data == NULL)
            return;

        gameroom_sync(data);

        status_set(session.online.status);

        if (a->cb)
            a->cb(a->args);

        free(data);
    }

    free(a);
}

void xmpp_iq_gameroom_setinfo(const char *mission_key,
                              f_gameroom_setinfo_cb cb, void *args)
{
    if (mission_key == NULL)
        return;

    struct cb_args *a = calloc(1, sizeof (struct cb_args));

    a->cb = cb;
    a->args = args;

    xmpp_send_iq_get(
        JID_MS(session.online.channel),
        xmpp_iq_gameroom_setinfo_cb, a,
        "<query xmlns='urn:cryonline:k01'>"
        " <gameroom_setinfo by_mission_key='1' mission_key='%s'/>"
        "</query>",
        mission_key);
}

