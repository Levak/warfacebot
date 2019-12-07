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
#include <wb_mission.h>
#include <wb_dbus.h>
#include <wb_cvar.h>
#include <wb_log.h>
#include <wb_lang.h>

#include <stdlib.h>
#include <string.h>

struct cb_args
{
    f_clan_create_cb cb;
    void *args;
};

static void xmpp_iq_clan_create_cb(const char *msg,
                                   enum xmpp_msg_type type,
                                   void *args)
{
    /* Answer:
       <iq from='masterserver@warface/xx' type='result'>
        <query xmlns='urn:cryonline:k01'>
         <clan_create>
          <clan name='xxxx' clan_id='xxx' description='base64xx'
                creation_date='xxxx' master='xxxx'
                clan_points='0' members='1'
                master_badge='xxx' master_stripe='xxx' master_mark='xx'>
           <clan_member_info nickname='xxx' profile_id='xxx'
                             experience='xxx' clan_points='0'
                             invite_date='xxx' clan_role='1'
                             jid='xxx@warface/GameClient' status='1'/>
          </clan>
         </clan_create>
        </query>
       </iq>
     */

    if (msg == NULL)
        return;

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
                        reason = LANG(clan_error_no_item);
                        break;
                    case 2:
                        reason = LANG(clan_error_invalid_name);
                        break;
                    case 3:
                        reason = LANG(clan_error_censored_name);
                        break;
                    case 4:
                        reason = LANG(clan_error_duplicate_name);
                        break;
                    case 5:
                        reason = LANG(error_already_in_a_clan);
                        break;
                    case 7:
                        reason = LANG(clan_error_reserved_name);
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }

        if (reason != NULL)
            eprintf("%s (%s)\n", LANG(error_clan_create), reason);
        else
            eprintf("%s (%i:%i)\n", LANG(error_clan_create), code, custom_code);
    }
    else
    {
        char *data = wf_get_query_content(msg);

        if (data == NULL)
            return;

        char *clan_node = get_info(data, "<clan ", "</clan>", NULL);

        clan_process_node(data);

        if (a->cb)
            a->cb(a->args);

        free(clan_node);
        free(data);
    }

    free(a);
}

void xmpp_iq_clan_create(const char *name, const char *description,
                         f_clan_create_cb cb, void *args)
{
    if (name == NULL || description == NULL)
        return;

    struct cb_args *a = calloc(1, sizeof (struct cb_args));

    a->cb = cb;
    a->args = args;

    char *desc_b64 = base64encode(description, strlen(description));

    xmpp_send_iq_get(
        JID_MS(session.online.channel),
        xmpp_iq_clan_create_cb, a,
        "<query xmlns='urn:cryonline:k01'>"
        " <clan_create clan_name='%s' description='%s'/>"
        "</query>",
        name,
        desc_b64);

    free(desc_b64);
}
