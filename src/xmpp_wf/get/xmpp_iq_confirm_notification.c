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
#include <wb_xmpp.h>
#include <wb_session.h>
#include <wb_xmpp_wf.h>
#include <wb_cvar.h>
#include <wb_log.h>

enum e_notif_result
{
    NOTIF_ACCEPT = 0,
    NOTIF_REFUSE = 1,
};

static void confirm(const char *notif_id,
                    enum e_notif_type notif_type,
                    enum e_notif_result result)
{
    xmpp_send_iq_get(
        JID_MS(session.online.channel),
        NULL, NULL,
        "<query xmlns='urn:cryonline:k01'>"
        " <confirm_notification>"
        "  <notif id='%s' type='%d'>"
        "   <confirmation result='%d' status='%d'"
        "                 location=''/>"
        "  </notif>"
        " </confirm_notification>"
        "</query>",
        notif_id,
        notif_type,
        result,
        session.online.status);
}

void xmpp_iq_confirm_notification(const char *notif)
{
    char *notif_id = get_info(notif, "id='", "'", NULL);
    enum e_notif_type notif_type = get_info_int(notif, "type='", "'", NULL);

    switch (notif_type)
    {
        /* Confirm consecutive logins */
        case NOTIF_MESSAGE:
            xprintf("Message\n");
            confirm(notif_id, notif_type, NOTIF_ACCEPT);
            break;

        case NOTIF_NEW_RANK:
        {
            int rank = get_info_int(notif, "new_rank='", "'", NULL);

            xprintf("New rank reached: %d\n", rank);
            confirm(notif_id, notif_type, NOTIF_ACCEPT);
            break;
        }

        case NOTIF_UNLOCK_MISSION:
        {
            char *mission = get_info(notif, "unlocked_mission='", "'", NULL);

            xprintf("Unlocked mission: %s\n", mission);
            confirm(notif_id, notif_type, NOTIF_ACCEPT);

            free(mission);
            break;
        }

        case NOTIF_CONS_LOGIN:
            xprintf("Getting consecutive reward\n");
            confirm(notif_id, notif_type, NOTIF_ACCEPT);
            break;

        case NOTIF_GIVE_ITEM:
        {
            char *item = get_info(notif, "name='", "'", NULL);
            char *offer = get_info(notif, "offer_type='", "'", NULL);

            xprintf("New item: %s (%s)\n", item, offer);
            confirm(notif_id, notif_type, NOTIF_ACCEPT);

            free(item);
            free(offer);
            break;
        }

        case NOTIF_GIVE_RANDOM_BOX:
            xprintf("Random box given\n");
            confirm(notif_id, notif_type, NOTIF_ACCEPT);
            break;

        case NOTIF_CLAN_PROMOTED:
            xprintf("Promoted to officer\n");
            confirm(notif_id, notif_type, NOTIF_ACCEPT);
            break;

        /* Accept any friend requests */
        case NOTIF_FRIEND_REQUEST:
        {
            char *initiator =
                get_info(notif, "initiator='", "'", NULL);

            if (!cvar.wb_postpone_friend_requests)
            {
                confirm(notif_id, notif_type,
                        cvar.wb_accept_friend_requests
                        ? NOTIF_ACCEPT
                        : NOTIF_REFUSE);
            }

            xprintf("Friend request from %s (%s)\n",
                    initiator,
                    cvar.wb_postpone_friend_requests
                    ? "postponed"
                    : cvar.wb_accept_friend_requests
                    ? "accepted"
                    : "refused");

            free(initiator);
            break;
        }

        /* Accept any clan invites only if we don't already have one */
        case NOTIF_CLAN_INVITE:
        {
            if (session.profile.clan.id == 0)
            {
                char *initiator =
                    get_info(notif, "initiator='", "'", NULL);
                char *clan_name =
                    get_info(notif, "clan_name='", "'", NULL);

                if (!cvar.wb_postpone_clan_invites)
                {
                    confirm(notif_id, notif_type,
                            cvar.wb_accept_clan_invites
                            ? NOTIF_ACCEPT
                            : NOTIF_REFUSE);
                }

                xprintf("%s invites us to his clan '%s' (%s)\n",
                        initiator,
                        clan_name,
                        cvar.wb_postpone_clan_invites
                        ? "postponed"
                        : cvar.wb_accept_clan_invites
                        ? "accepted"
                        : "refused");

                free(initiator);
                free(clan_name);
            }
            break;
        }

        case NOTIF_CLAN_INVITE_RESULT:
        {
            char *nick = get_info(notif, "nickname='", "'", NULL);
            int result = get_info_int(notif, "result='", "'", NULL);

            switch (result)
            {
                case 0:
                    break;
                case 1:
                    xprintf("%s rejected the clan invitation\n", nick);
                    break;
                default:
                    xprintf("Failed to invite %s to clan (code: %d)\n",
                           nick, result);
                    break;
            }

            confirm(notif_id, notif_type, NOTIF_ACCEPT);

            free(nick);
            break;
        }

        /* Old fashion peer_status_update */
        case NOTIF_STATUS_UPDATE:
        {
            char *jid = get_info(notif, "jid='", "'", NULL);
            char *nick = get_info(notif, "nickname='", "'", NULL);
            char *pid = get_info(notif, "profile_id='", "'", NULL);
            int status = get_info_int(notif, "status='", "'", NULL);
            int exp = get_info_int(notif, "experience='", "'", NULL);

            if (status <= STATUS_OFFLINE)
                jid = NULL;

            struct friend * f = friend_list_add(jid, nick, pid, status, exp);
            xmpp_iq_peer_status_update(f);

            free(jid);
            free(nick);
            free(pid);

            break;
        }

        case NOTIF_ANNOUNCEMENT:
        case NOTIF_ACHIEVEMENT:
            confirm(notif_id, notif_type, NOTIF_ACCEPT);
            break;

        default:
            break;
    }

    free(notif_id);
}
