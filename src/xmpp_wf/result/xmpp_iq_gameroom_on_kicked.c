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

#include <string.h>

#include <wb_tools.h>
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>
#include <wb_session.h>
#include <wb_quickplay.h>
#include <wb_log.h>
#include <wb_dbus.h>

static void xmpp_iq_gameroom_on_kicked_cb(const char *msg_id,
                                          const char *msg,
                                          void *args)
{
    /* Answer
       <iq from='masterserver@warface/pvp_pro_4' type='get'>
        <query xmlns='urn:cryonline:k01'>
         <gameroom_on_kicked reason='1'/>
        </query>
       </iq>
    */

    if (strstr(msg, "from='masterserver@warface") == NULL)
        return;

    char *data = wf_get_query_content(msg);

    if (data == NULL)
        return;

    int reason = get_info_int(data, "reason='", "'", NULL);
    const char *reason_str = "Unknown";

    switch (reason)
    {
        case KICK_NOREASON:
            reason_str = "Kicked by master";
            break;
        case KICK_INACTIVITY:
            reason_str = "Kicked for inactivity";
            break;
        case KICK_VOTE:
            reason_str = "Vote-kicked";
            break;
        case KICK_RANK:
            reason_str = "Rank too high";
            break;
        case KICK_CLAN_ROOM:
            reason_str = "Not part of the clanwar";
            break;
        case KICK_CHEATING:
            reason_str = "Kicked for suspected of cheating";
            break;
        case KICK_GAME_VERSION:
            reason_str = "Wrong game version";
            break;
        case KICK_NOTOKEN:
            reason_str = "Not access token left";
            break;
        case KICK_MATCHMAKING:
            reason_str = "Matchmaking";
            break;
        case KICK_RATING_END:
            reason_str = "End of ranked match";
            break;
        default:
            break;
    }

    xprintf("Kicked from the room (reason: %s)\n", reason_str);

#ifdef DBUS_API
    dbus_api_emit_room_kicked(reason);
#endif /* DBUS_API */

    xmpp_iq_player_status(STATUS_ONLINE | STATUS_LOBBY);
    xmpp_presence(session.gameroom.jid, XMPP_PRESENCE_LEAVE, NULL, NULL);
    free(session.gameroom.group_id);
    session.gameroom.group_id = NULL;
    free(session.gameroom.jid);
    session.gameroom.jid = NULL;
    free(session.gameroom.room_id);
    session.gameroom.room_id = NULL;

    quickplay_free();

    free(data);
}

void xmpp_iq_gameroom_on_kicked_r(void)
{
    qh_register("gameroom_on_kicked", 1, xmpp_iq_gameroom_on_kicked_cb, NULL);
}
