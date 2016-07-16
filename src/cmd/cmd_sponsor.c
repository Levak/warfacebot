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

#include <wb_session.h>
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>
#include <wb_log.h>

void cmd_sponsor(enum sponsor_type st)
{
    const char *sponsor = NULL;

    switch (st)
    {
        case SPONSOR_WEAPON:
            sponsor = "Weapon";
            break;
        case SPONSOR_OUTFIT:
            sponsor = "Outfit";
            break;
        case SPONSOR_EQUIPMENT:
            sponsor = "Equipment";
            break;
        default:
            return;
    }

    xprintf("Now using the %s sponsor\n", sponsor);

    xmpp_send_iq_get(
        JID_MS(session.online.channel),
        NULL, NULL,
        "<query xmlns='urn:cryonline:k01'>"
        " <persistent_settings_set>"
        "  <settings>"
        "   <sponsor id='%d'/>"
        "  </settings>"
        " </persistent_settings_set>"
        "</query>",
        st);
}

void cmd_sponsor_wrapper(const char *sponsor)
{
    enum sponsor_type st;

    if (0 == strcasecmp(sponsor, "weapon"))
        st = SPONSOR_WEAPON;
    else if (0 == strcasecmp(sponsor, "outfit"))
        st = SPONSOR_OUTFIT;
    else if (0 == strcasecmp(sponsor, "equipment"))
        st = SPONSOR_EQUIPMENT;
    else
    {
        eprintf("Unknown sponsor '%s' "
                "(should be one of weapon/outfit/equipment)\n",
                sponsor);
        return;
    }

    cmd_sponsor(st);
}
