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
    const struct clanmate *c = (const struct clanmate *) args;

    if (type & XMPP_TYPE_ERROR)
    {
        int status = STATUS_OFFLINE;
        char *nick = strdup(c->nickname);
        char *pid = strdup(c->profile_id);
        unsigned int exp = c->experience;
        unsigned int cp = c->clan_points;
        unsigned int role = c->clan_role;

        clanmate_list_update(NULL, nick, pid, status, exp, cp, role);

#ifdef DBUS_API
        dbus_api_emit_status_update(nick, status, exp, cp);
#endif /* DBUS_API */

        free(nick);
        free(pid);
    }
}

/*
 * Type: f_list_callback
 */
void xmpp_iq_peer_clan_member_update_clanmate(const struct clanmate *c, void *args)
{
    if (c->jid)
        xmpp_iq_peer_clan_member_update(c);
}

void xmpp_iq_peer_clan_member_update(const struct clanmate *c)
{
    if (c == NULL || c->jid == NULL)
        return;

    t_uid id;

    idh_generate_unique_id(&id);
    idh_register(&id, 0, xmpp_iq_peer_clan_member_update_cb, (void *) c);

    /* Inform to our clanmates our status */
    send_stream_format(session.wfs,
                       "<iq id='%s' to='%s' type='get'>"
                       " <query xmlns='urn:cryonline:k01'>"
                       "  <peer_clan_member_update nickname='%s'"
                       "     profile_id='%s' status='%u' experience='%u'"
                       "     place_token='' place_info_token=''"
                       "     clan_points='%u' clan_role='%u'/>"
                       " </query>"
                       "</iq>",
                       &id, c->jid,
                       session.nickname, session.profile_id,
                       session.status, session.experience,
                       session.clan_points, session.clan_role);
}
