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
#include <wb_xml.h>
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>

#include <stdlib.h>
#include <string.h>

struct cb_args
{
    f_profile_info_get_status_cb cb;
    void *args;
};

static void xmpp_iq_profile_info_get_status_cb(const char *msg,
                                               enum xmpp_msg_type type,
                                               void *args)
{
    /* Answer:
       <iq from='k01.warface' type='result'>
        <query xmlns='urn:cryonline:k01'>
         <profile_info_get_status nickname='xxxx'>
          <profile_info>
           <info nickname='xxxx' online_id='xxxx@warface/GameClient'
                 status='33' profile_id='xxx' user_id='xxxxx'
                 rank='xx' tags='' ip_address='xxx.xxx.xxx.xxx'
                 login_time='xxxxxxxxxxx'/>
          </profile_info>
         </profile_info_get_status>
        </query>
       </iq>
     */

    struct cb_args *a = (struct cb_args *) args;
    char *info = NULL;

    if (type ^ XMPP_TYPE_ERROR)
    {
        info = get_info(msg, "<info", "/>", NULL);
    }

    if (a->cb)
        a->cb(info, a->args);

    free(info);
    free(a);
}

void xmpp_iq_profile_info_get_status(const char *nickname,
                                     f_profile_info_get_status_cb f,
                                     void *args)
{
    char *nick = strdup(nickname);
    struct cb_args *a = calloc(1, sizeof (struct cb_args));

    a->cb = f;
    a->args = args;

    xmpp_send_iq_get(
        JID_K01,
        xmpp_iq_profile_info_get_status_cb, a,
        "<query xmlns='urn:cryonline:k01'>"
        "<profile_info_get_status nickname='%s'/>"
        "</query>",
        xml_serialize_inplace(&nick));

    free(nick);
}
