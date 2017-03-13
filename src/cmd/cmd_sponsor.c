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

#include <wb_session.h>
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>
#include <wb_log.h>
#include <wb_cmd.h>
#include <wb_lang.h>

void cmd_sponsor(enum sponsor_type st)
{
    const char *sponsor = NULL;

    switch (st)
    {
        case SPONSOR_WEAPON:
            sponsor = LANG(console_sponsor_weapon);
            break;
        case SPONSOR_OUTFIT:
            sponsor = LANG(console_sponsor_outfit);
            break;
        case SPONSOR_EQUIPMENT:
            sponsor = LANG(console_sponsor_equipment);
            break;
        default:
            return;
    }

    {
        char *s = LANG_FMT(console_sponsor, sponsor);
        xprintf("%s", s);
        free(s);
    }

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
        eprintf("%s: '%s' "
                "(%s weapon/outfit/equipment)",
                LANG(console_sponsor_unknown),
                sponsor,
                LANG(console_sponsor_should_be));
        return;
    }

    cmd_sponsor(st);
}

int cmd_sponsor_completions(struct list *l)
{
    list_add(l, strdup("weapon"));
    list_add(l, strdup("outfit"));
    list_add(l, strdup("equipment"));

    return 1;
}
