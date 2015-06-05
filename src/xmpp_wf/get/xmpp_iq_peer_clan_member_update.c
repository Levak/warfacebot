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

void xmpp_iq_peer_clan_member_update_clanmate(struct clanmate *f, void *args)
{
    if (f->jid)
        xmpp_iq_peer_clan_member_update(f->jid);
}

void xmpp_iq_peer_clan_member_update(const char *to_jid)
{
    /* Inform to our clanmates our status */
    send_stream_format(session.wfs,
                       "<iq to='%s' type='get'>"
                       " <query xmlns='urn:cryonline:k01'>"
                       "  <peer_clan_member_update nickname='%s'"
                       "     profile_id='%s' status='%u' experience='%u'"
                       "     place_token='' place_info_token=''"
                       "     clan_points='%u' clan_role='%u'/>"
                       " </query>"
                       "</iq>",
                       to_jid,
                       session.nickname, session.profile_id,
                       session.status, session.experience,
                       session.clan_points, session.clan_role);
}
