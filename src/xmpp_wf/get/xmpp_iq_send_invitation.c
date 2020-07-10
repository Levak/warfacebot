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
#include <wb_log.h>
#include <wb_lang.h>

#include <stdlib.h>

struct cb_args
{
    f_send_invitation_cb cb;
    void *args;
};

static void xmpp_iq_send_invitation_cb(const char *msg,
                                       enum xmpp_msg_type type,
                                       void *args)
{
    /* Answer :
       <iq to='masterserver@warface/pve_2' type='get'>
        <query xmlns='urn:cryonline:k01'>
         <send_invitation type='xx' .../>
        </query>
       </iq>
     */

    struct cb_args *a = (struct cb_args *) args;
    int error_code;

    if (type & XMPP_TYPE_ERROR)
    {
        const char *reason = NULL;

        int code = get_info_int(msg, "code='", "'", NULL);
        int custom_code = get_info_int(msg, "custom_code='", "'", NULL);

        if (custom_code == 0)
            error_code = code;
        else
            error_code = custom_code;

        switch (code)
        {
            case 8:
                switch (custom_code)
                {
                    case 2:
                        reason = LANG(error_already_sent);
                        break;
                    case 4:
                        reason = LANG(error_already_in_fl);
                        break;
                    case 5:
                        reason = LANG(error_already_in_a_clan);
                        break;
                    case 9:
                        reason = LANG(error_user_not_connected);
                        break;
                    case 10:
                        reason = LANG(error_not_in_a_clan);
                        break;
                    case 12:
                        reason = LANG(error_friendlist_full);
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
                    LANG(error_send_invitation),
                    reason);
        else
            eprintf("%s (%i:%i)",
                    LANG(error_send_invitation),
                    code,
                    custom_code);
    }
    else
    {
        error_code = 0;
    }

    if (a->cb)
        a->cb(error_code, a->args);

    free(a);
}

void xmpp_iq_send_invitation(const char *nickname,
                             enum notif_type type,
                             f_send_invitation_cb cb, void *args)
{
    struct cb_args *a = calloc(1, sizeof (struct cb_args));

    a->cb = cb;
    a->args = args;

    xmpp_send_iq_get(
        JID_MS(session.online.channel),
        xmpp_iq_send_invitation_cb, a,
        "<query xmlns='urn:cryonline:k01'>"
        "<send_invitation target='%s' type='%d'/>"
        "</query>",
        nickname,
        type);
}
