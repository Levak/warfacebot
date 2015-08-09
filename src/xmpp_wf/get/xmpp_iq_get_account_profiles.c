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
#include <wb_game.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void xmpp_iq_get_account_profiles_cb(const char *msg, void *args)
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

    if (xmpp_is_error(msg))
    {
        fprintf(stderr, "Failed to get account profiles\nReason: ");

        int code = get_info_int(msg, "code='", "'", NULL);
        int custom_code = get_info_int(msg, "custom_code='", "'", NULL);

        switch (code)
        {
            case 8:
                switch (custom_code)
                {
                    case 1:
                        fprintf(stderr, "Game version mismatch (%s)\n",
                                game_version_get());
                        return;
                    default:
                        fprintf(stderr, "Invalid user_id or active_token\n");
                        return;
                }
            default:
                fprintf(stderr, "Unknown\n");
                return;
        }
    }

    free(session.profile_id);
    free(session.nickname);

    session.profile_id = get_info(msg, "profile id='", "'", "PROFILE ID");
    session.nickname = get_info(msg, "nickname='", "'", "NICKNAME");

    if (!session.profile_id)
        xmpp_iq_create_profile();
    else
        xmpp_iq_join_channel(NULL, NULL, NULL);
}

void xmpp_iq_get_account_profiles(void)
{
    t_uid id;

    idh_generate_unique_id(&id);
    idh_register(&id, 0, xmpp_iq_get_account_profiles_cb, NULL);

    /* Get CryOnline profile */
    send_stream_format(session.wfs,
                       "<iq id='%s' to='ms.warface' type='get'>"
                       " <query xmlns='urn:cryonline:k01'>"
                       "  <get_account_profiles version='%s'"
                       "    user_id='%s' token='%s'/>"
                       " </query>"
                       "</iq>",
                       &id, game_version_get(),
                       session.online_id, session.active_token);
}
