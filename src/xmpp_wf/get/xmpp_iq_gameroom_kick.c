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

#include <stdlib.h>

void xmpp_iq_gameroom_kick(unsigned int profile_id,
                           f_id_callback cb, void *args)
{
    t_uid id;

    idh_generate_unique_id(&id);
    idh_register(&id, 0, cb, args);

    xmpp_send_iq_get(
        JID_MS(session.online.channel),
        cb, args,
        "<query xmlns='urn:cryonline:k01'>"
        " <gameroom_kick target_id='%u'/>"
        "</query>",
        profile_id);
}

