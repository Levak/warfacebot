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
    char *data = wf_get_query_content(msg);

    if (type & XMPP_TYPE_ERROR)
    {
        eprintf("Failed to create profile\nReason: ");

        int code = get_info_int(msg, "code='", "'", NULL);
        int custom_code = get_info_int(msg, "custom_code='", "'", NULL);

        switch (code)
        {
            case 503:
                eprintf("Invalid channel (%s)\n", session.online.channel);
                return;
            case 8:
                switch (custom_code)
                {
                    case 0:
                        eprintf("Invalid token (%s) or userid (%s)\n",
                                session.online.active_token,
                                session.online.id);
                        return;
                    case 1:
                        eprintf("Invalid profile_id (%s)\n",
                                session.profile.id);
                        return;
                    case 2:
                        eprintf("Game version mismatch (%s)\n",
                                cvar.game_version);
                        return;
                    case 3:
                        eprintf("Banned\n");
                        break;
                    default:
                        eprintf("Unknown\n");
                        return;
                }
            default:
                eprintf("Unknown\n");
                return;
        }

        return;
    }

    session.profile.id = get_info(data, "profile_id='", "'", "PROFILE ID");
    session.profile.nickname = get_info(data, "nick='", "'", "NICKNAME");

#ifdef DBUS_API
    dbus_api_setup();
#endif

    /* Ask for today's missions list */
    mission_list_update(NULL, NULL);

    free(data);
}

void xmpp_iq_create_profile(void)
{
    xmpp_send_iq_get(
        JID_K01,
        xmpp_iq_create_profile_cb, NULL,
        "<query xmlns='urn:cryonline:k01'>"
        "<create_profile version='%s' region_id='%s'"
        "                user_id='%s' token='%s'"
        "                nickname='' resource='%s'/>"
        "</query>",
        cvar.game_version,
        cvar.online_region_id,
        session.online.id,
        session.online.active_token,
        session.online.channel);
}
