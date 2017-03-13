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
#include <wb_cvar.h>
#include <wb_log.h>
#include <wb_lang.h>

#include <stdlib.h>

static void xmpp_iq_gameroom_loosemaster_cb(const char *msg_id,
                                            const char *msg,
                                            void *args)
{
    /* Answer:
       <iq from='masterserver@warface/pvp_pro_4' type='get'>
        <query xmlns='urn:cryonline:k01'>
         <gameroom_loosemaster time='10'/>
        </query>
       </iq>
     */

    unsigned int time = get_info_int(msg, "time='", "'", NULL);

    if (!cvar.wb_auto_start)
    {
        char *s = LANG_FMT(gameroom_loosemaster, time);
        xprintf("%s", s);
        free(s);
        return;
    }
    else
    {
        xprintf("%s", LANG(gameroom_autostart));
    }

    xmpp_iq_gameroom_askserver(NULL, NULL);
}

void xmpp_iq_gameroom_loosemaster_r(void)
{
    qh_register("gameroom_loosemaster", 1, xmpp_iq_gameroom_loosemaster_cb, NULL);
}
