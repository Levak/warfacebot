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

#include <wb_session.h>
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>

struct cb_args
{
    f_preinvite_invite_cb cb;
    void *args;
};

static void xmpp_iq_preinvite_invite_cb(const char *msg,
                                        enum xmpp_msg_type type,
                                        void *args)
{
    struct cb_args *a = (struct cb_args *) args;

    if (type ^ XMPP_TYPE_ERROR)
    {
        if (a->cb)
            a->cb(a->args);
    }

    free(a);
}

void xmpp_iq_preinvite_invite(const char *online_id,
                              const char *uid,
                              f_preinvite_invite_cb cb,
                              void *args)
{
    struct cb_args *a = calloc(1, sizeof (struct cb_args));

    a->cb = cb;
    a->args = args;

    xmpp_send_iq_get(
        JID(online_id),
        xmpp_iq_preinvite_invite_cb, a,
        "<query xmlns='urn:cryonline:k01'>"
        " <preinvite_invite from='%s' uid='%s'"
        " ms_resource='%s' channel_type='%s'"
        " mission_id='%s' clan_name='%s' experience='%u'"
        " badge='%u' mark='%u' stripe='%u'"
        " onlineId='%s' profileId='%s' is_online='1'/>"
        "</query>",
        session.profile.nickname,
        uid,
        session.online.channel,
        session.online.channel_type,
        session.quickplay.mission_id ? session.quickplay.mission_id : "",
        session.profile.clan.name ? session.profile.clan.name : "",
        session.profile.experience,
        session.profile.banner.badge,
        session.profile.banner.mark,
        session.profile.banner.stripe,
        session.online.id,
        session.profile.id);
}
