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
#include <wb_clanmate.h>
#include <wb_dbus.h>

static void xmpp_iq_peer_clan_member_update_cb(const char *msg,
                                               enum xmpp_msg_type type,
                                               void *args)
{
    /* If user is not reachable, it means he disconnected */
    if (type & XMPP_TYPE_ERROR)
    {
        struct clanmate *cp = (struct clanmate *) args;

        cp->status = STATUS_OFFLINE;

        clanmate_list_update(NULL,
                             cp->nickname,
                             cp->profile_id,
                             STATUS_OFFLINE,
                             cp->experience,
                             cp->clan_points,
                             cp->clan_role);

#ifdef DBUS_API
        dbus_api_emit_status_update(cp->nickname, cp->status,
                                    cp->experience, cp->clan_points);
#endif /* DBUS_API */

        clanmate_free(cp);
    }
}

/*
 * Type: f_list_callback
 */
void xmpp_iq_peer_clan_member_update_clanmate(const struct clanmate *c, void *args)
{
    if (c->jid)
    {
        struct clanmate *cp = clanmate_new(c->jid, c->nickname, c->profile_id,
                                           c->status, c->experience,
                                           c->clan_points, c->clan_role);

        xmpp_iq_peer_clan_member_update(cp);
    }
}

void xmpp_iq_peer_clan_member_update(const struct clanmate *c)
{
    if (c == NULL || c->jid == NULL)
        return;

    t_uid id;

    idh_generate_unique_id(&id);
    idh_register(&id, 0, xmpp_iq_peer_clan_member_update_cb, (void *) c);

    /* Inform to our clanmates our status */
    send_stream_format(
        session.wfs,
        "<iq id='%s' to='%s' type='get'>"
        " <query xmlns='urn:cryonline:k01'>"
        "  <peer_clan_member_update nickname='%s'"
        "     profile_id='%s' status='%u' experience='%u'"
        "     place_token='@%s' place_info_token='@%s'"
        "     mode_info_token='%s' mission_info_token='%s'/>"
        "     clan_points='%u' clan_role='%u'/>"
        " </query>"
        "</iq>",
        &id, c->jid,
        session.profile.nickname,
        session.profile.id,
        session.online.status,
        session.profile.experience,
        session.online.place_token,
        session.online.place_info_token,
        session.online.mode_info_token,
        session.online.mission_info_token,
        session.profile.clan.points,
        session.profile.clan.role);
    }
