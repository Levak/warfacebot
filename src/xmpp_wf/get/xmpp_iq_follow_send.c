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

#include <stdlib.h>
#include <wb_log.h>

struct cb_args
{
    char *nickname;
};

static void xmpp_iq_follow_send_cb(const char *msg,
                                   enum xmpp_msg_type type,
                                   void *args)
{
    /* Answer :
       <iq to='xxx@warface/GameClient' type='result'>
        <query xmlns='urn:cryonline:k01'>
         <follow_send/>
        </query>
       </iq>
     */

    struct cb_args *a = (struct cb_args *) args;

    if (type & XMPP_TYPE_ERROR)
    {
        invitation_complete(a->nickname, NULL, INVIT_USER_OFFLINE, 1);
    }

    free(a->nickname);
    free(a);
}

void xmpp_iq_follow_send(const char *nickname,
                         const char *online_id,
                         f_invitation_result_cb cb,
                         void *args)
{
    if (nickname == NULL || online_id == NULL)
        return;

    struct cb_args *a = calloc(1, sizeof (struct cb_args));

    a->nickname = strdup(nickname);

    invitation_register(nickname, 1, cb, args);

    xmpp_send_iq_get(
        JID(online_id),
        xmpp_iq_follow_send_cb, a,
        "<query xmlns='urn:cryonline:k01'>"
        " <follow_send nickname='%s' profile_id='%s'/>"
        "</query>",
        session.profile.nickname,
        session.profile.id);
}
