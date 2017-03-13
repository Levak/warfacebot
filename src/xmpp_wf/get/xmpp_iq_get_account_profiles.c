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
#include <wb_dbus.h>
#include <wb_cvar.h>
#include <wb_log.h>
#include <wb_lang.h>

#include <stdlib.h>
#include <string.h>

static void xmpp_iq_get_account_profiles_cb(const char *msg,
                                            enum xmpp_msg_type type,
                                            void *args)
{
    /* Answer :
       <iq from="masterserver@warface/pve_12" type="result">
         <query xmlns="urn:cryonline:k01">
           <get_account_profiles>
             <profile id="XXX" nickname="XXX"/>
           </get_account_profiles>
         </query>
       </iq>
    */

    if (type & XMPP_TYPE_ERROR)
    {
        if (msg == NULL)
            msg = "";

        const char *reason = NULL;
        int code = get_info_int(msg, "code='", "'", NULL);
        int custom_code = get_info_int(msg, "custom_code='", "'", NULL);

        switch (code)
        {
            case 0:
                reason = LANG(error_timeout);
                break;
            case 8:
                switch (custom_code)
                {
                    case 1:
                        reason = LANG(error_game_version);
                        break;
                    default:
                        reason = LANG(error_invalid_login);
                        break;
                }
                break;
            case 1002:
                reason = LANG(error_already_logged_in);
                break;
            default:
                break;
        }

        if (reason != NULL)
            eprintf("%s (reason: %s)",
                    LANG(error_get_account_profiles),
                    reason);
        else
            eprintf("%s (code %d:%d)",
                    LANG(error_get_account_profiles),
                    code,
                    custom_code);

        return;
    }

    free(session.profile.id);
    free(session.profile.nickname);

    session.profile.id = get_info(msg, "profile id='", "'",
                                  LANG(profile_id));
    session.profile.nickname = get_info(msg, "nickname='", "'",
                                        LANG(nickname));

    if (session.profile.id == NULL)
        xmpp_iq_create_profile();
    else
    {
        xmpp_iq_join_channel(session.online.channel, NULL, NULL);
#ifdef DBUS_API
        dbus_api_setup();
#endif
    }
}

void xmpp_iq_get_account_profiles(void)
{
    /* Get CryOnline profile */
    xmpp_send_iq_get(
        JID_MS(session.online.channel),
        xmpp_iq_get_account_profiles_cb, NULL,
        "<query xmlns='urn:cryonline:k01'>"
        " <get_account_profiles version='%s'"
        "   user_id='%s' token='%s'/>"
        "</query>",
        cvar.game_version,
        session.online.id,
        session.online.active_token);
}
