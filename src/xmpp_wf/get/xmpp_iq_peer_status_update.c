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
#include <wb_stream.h>
#include <wb_session.h>
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>
#include <wb_friend.h>
#include <wb_dbus.h>

static void xmpp_iq_peer_status_update_cb(const char *msg,
                                          enum xmpp_msg_type type,
                                          void *args)
{
    struct friend *f = (struct friend *) args;

    if (type & XMPP_TYPE_ERROR)
    {
        f->status = STATUS_OFFLINE;

        friend_list_update(NULL,
                           f->nickname,
                           f->profile_id,
                           f->status,
                           f->experience);

#ifdef DBUS_API
        dbus_api_emit_status_update(f->nickname, f->status,
                                    f->experience, 0);
#endif /* DBUS_API */
    }

    friend_free(f);
}

static void xmpp_iq_peer_status_update_(struct friend *f)
{
    t_uid id;

    idh_generate_unique_id(&id);
    idh_register(&id, 0, xmpp_iq_peer_status_update_cb, (void *) f);

    /* Inform to our friends our status */
    send_stream_format(session.wfs,
                       "<iq id='%s' to='%s' type='get'>"
                       " <query xmlns='urn:cryonline:k01'>"
                       "  <peer_status_update nickname='%s' profile_id='%s'"
                       "     status='%u' experience='%u'"
                       "     place_token='@%s' place_info_token='@%s'"
                       "     mode_info_token='%s' mission_info_token='%s'/>"
                       " </query>"
                       "</iq>",
                       &id, f->jid,
                       session.profile.nickname,
                       session.profile.id,
                       session.online.status,
                       session.profile.experience,
                       session.online.place_token,
                       session.online.place_info_token,
                       session.online.mode_info_token,
                       session.online.mission_info_token);
}

/*
  Type: f_list_callback
 */
void xmpp_iq_peer_status_update_friend(const struct friend *f, void *args)
{
    if (f == NULL || f->jid == NULL)
        return;

    struct friend *fr = friend_new(f->jid, f->nickname, f->profile_id,
                                   f->status, f->experience);

    xmpp_iq_peer_status_update_(fr);
}

void xmpp_iq_peer_status_update(const struct friend *f)
{
    xmpp_iq_peer_status_update_friend(f, NULL);
}
