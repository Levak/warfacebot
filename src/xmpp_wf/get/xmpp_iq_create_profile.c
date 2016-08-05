/**
 * WarfaceBot, a blind XMPP client for Warface (FPS)
 * Copyright (C) 2015, 2016 Levak Borok <levak92@gmail.com>
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
#include <wb_mission.h>
#include <wb_dbus.h>
#include <wb_cvar.h>
#include <wb_log.h>

#include <stdlib.h>
#include <string.h>

static void xmpp_iq_create_profile_cb(const char *msg,
                                      enum xmpp_msg_type type,
                                      void *args)
{
    if (msg == NULL)
        return;

    if (type & XMPP_TYPE_ERROR)
    {
        const char *reason = NULL;

        int code = get_info_int(msg, "code='", "'", NULL);
        int custom_code = get_info_int(msg, "custom_code='", "'", NULL);

        switch (code)
        {
            case 1006:
                reason = "QoS limit reached";
                break;
            case 503:
                reason = "Invalid channel";
                break;
            case 8:
                switch (custom_code)
                {
                    case 0:
                        reason = "Invalid token or userid";
                        break;
                    case 2:
                        reason = "Invalid nickname";
                        break;
                    case 4:
                        reason = "Game version mismatch";
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }

        if (reason != NULL)
            eprintf("Failed to create profile (%s)\n", reason);
        else
            eprintf("Failed to create profile (%i:%i)\n", code, custom_code);

        return;
    }

    /* We should be here only once, else something is strange */
    static int creation = 0;
    if (creation == 0)
    {
        creation = 1;
        xprintf("Created new profile\n");

        /* Get back to the original login workflow */
        xmpp_iq_get_account_profiles();
    }
    else
    {
        eprintf("Error while creating profile\n");
    }
}

void xmpp_iq_create_profile(void)
{
    xmpp_send_iq_get(
        JID_K01,
        xmpp_iq_create_profile_cb, NULL,
        "<query xmlns='urn:cryonline:k01'>"
        "<create_profile"
        " hw_id='%d' build_type='--release'"
        " version='%s' region_id='%s'"
        " user_id='%s' token='%s'"
        " nickname='' resource='%s'/>"
        "</query>",
        cvar.game_hwid,
        cvar.game_version,
        cvar.online_region_id,
        session.online.id,
        session.online.active_token,
        session.online.channel);
}
