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

#include <wb_stream.h>
#include <wb_session.h>
#include <wb_tools.h>

#include <unistd.h>

void xmpp_close(void)
{
    if (session.wfs < 0)
        return;

    /* Close stream */
    {
        char *s;
        FORMAT(s,
               "<iq to='%s' type='get'>"
               "<query xmlns='urn:cryonline:k01'>"
               "<player_status prev_status='%u' new_status='%u' to='%s'/>"
               "</query>"
               "</iq>",
               session.online.jid.k01,
               session.online.status,
               STATUS_LEFT,
               session.online.channel ? session.online.channel : "");
        stream_send_msg(session.wfs, s);
        free(s);
    }

    stream_send_msg(session.wfs, "</stream:stream>");
    stream_flush(session.wfs);
    close(session.wfs);

    session.wfs = -1;

#ifdef USE_TLS
    tls_close();
    tls_free();
#endif
}
