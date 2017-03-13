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
#include <wb_clanmate.h>
#include <wb_dbus.h>

static void xmpp_iq_peer_clan_member_update_cb(const char *msg,
                                               enum xmpp_msg_type type,
                                               void *args)
{
    struct clanmate *cp = (struct clanmate *) args;

    /* If user is not reachable, it means he disconnected */
    if (type & XMPP_TYPE_ERROR)
    {
        cp->status = STATUS_OFFLINE;

        clanmate_list_update(NULL,
                             cp->nickname,
                             cp->profile_id,
                             cp->status,
                             cp->experience,
                             cp->clan_points,
                             cp->clan_role);

#ifdef DBUS_API
        dbus_api_emit_status_update(cp->nickname, cp->status,
                                    cp->experience, cp->clan_points);
#endif /* DBUS_API */

    }

    clanmate_free(cp);
}

#define NULL_CHECK(X) ((X) != NULL ? (X) : "")
static void xmpp_iq_peer_clan_member_update_(struct clanmate *c)
{
    /* Inform to our clanmates our status */
    xmpp_send_iq_get(
        JID(c->jid),
        xmpp_iq_peer_clan_member_update_cb, c,
        "<query xmlns='urn:cryonline:k01'>"
        " <peer_clan_member_update nickname='%s'"
        "    profile_id='%s' status='%u' experience='%u'"
        "    place_token='%s' place_info_token='%s'"
        "    mode_info_token='%s' mission_info_token='%s'"
        "    clan_points='%u' clan_role='%u'/>"
        "</query>",
        session.profile.nickname,
        session.profile.id,
        session.online.status,
        session.profile.experience,
        NULL_CHECK(session.online.place_token),
        NULL_CHECK(session.online.place_info_token),
        NULL_CHECK(session.online.mode_info_token),
        NULL_CHECK(session.online.mission_info_token),
        session.profile.clan.points,
        session.profile.clan.role);
}
#undef NULL_CHECK

/*
 * Type: f_list_callback
 */
void xmpp_iq_peer_clan_member_update_clanmate(const struct clanmate *c, void *args)
{
    if (c == NULL || c->jid == NULL)
        return;

    struct clanmate *cp = clanmate_new(c->jid, c->nickname, c->profile_id,
                                       c->status, c->experience,
                                       c->clan_points, c->clan_role);

    xmpp_iq_peer_clan_member_update_(cp);
}

void xmpp_iq_peer_clan_member_update(const struct clanmate *c)
{
    xmpp_iq_peer_clan_member_update_clanmate(c, NULL);
}
