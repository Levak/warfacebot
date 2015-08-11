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
#include <wb_geoip.h>
#include <wb_stream.h>
#include <wb_session.h>
#include <wb_xml.h>
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

struct cb_args
{
    f_peer_player_info_cb cb;
    void *args;
};

static void xmpp_iq_peer_player_info_cb(const char *msg, void *args)
{
    /* Answer:
       <iq type="result" to="xxxxx@warface/GameClient">
        <query xmlns="urn:cryonline:k01">
         <peer_player_info online_id="xxxx@warface/GameClient"
                           nickname="xxxxx" [....]
                           clan_name="xxxxxx" [...]/>
        </query>
       </iq>
     */

    struct cb_args *a = (struct cb_args *) args;

    if (xmpp_is_error(msg))
    {
        if (a->cb)
            a->cb(NULL, a->args);

        free(a);
        return;
    }

    char *info = get_info(msg, "k01\">", "</query>", NULL);

    if (a->cb)
        a->cb(info, a->args);

    free(a);
    free(info);
}

void xmpp_iq_peer_player_info(const char *online_id,
                              f_peer_player_info_cb f, void *args)
{
    struct cb_args *a = calloc(1, sizeof (struct cb_args));

    a->cb = f;
    a->args = args;

    t_uid id;

    idh_generate_unique_id(&id);
    idh_register(&id, 0, xmpp_iq_peer_player_info_cb, a);

    send_stream_format(session.wfs,
                       "<iq to='%s' type='get' id='%s'>"
                       "<query xmlns='urn:cryonline:k01'>"
                       "<peer_player_info/>"
                       "</query>"
                       "</iq>",
                       &id);
}
