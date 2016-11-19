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
#include <wb_xmpp_wf.h>
#include <wb_log.h>

#include <stdlib.h>

static struct list *pending_invitations = NULL;

struct invitation
{
    char *nickname;
    f_invitation_result_cb cb;
    void *args;
};

static int invitation_cmp(struct invitation *i, const char *nickname)
{
    return strcmp(i->nickname, nickname);
}

static void invitation_free(struct invitation *i)
{
    if (i->cb != NULL)
        i->cb(NULL, INVIT_EXPIRED, i->args);

    free(i->nickname);
    free(i);
}

void invitation_register(const char *nickname,
                         f_invitation_result_cb cb,
                         void *args)
{
    if (nickname == NULL)
        return;

    if (pending_invitations == NULL)
    {
        pending_invitations = list_new((f_list_cmp) invitation_cmp,
                                       (f_list_free) invitation_free);
    }

    struct invitation *old_invit = list_get(pending_invitations, nickname);

    if (old_invit != NULL)
    {
        return;
    }

    struct invitation *new_invit = calloc(1, sizeof (struct invitation));

    new_invit->nickname = strdup(nickname);
    new_invit->cb = cb;
    new_invit->args = args;

    list_add(pending_invitations, new_invit);
}

static const char *_get_invitation_failure(int is_follow,
                                           enum invitation_result r)
{
    switch (r)
    {
        case INVIT_REJECTED:
            return "Rejected";
        case INVIT_PENDING:
            return "Already pending";
        case INVIT_AUTOREJECT:
            return "Autoreject";
        case INVIT_DUPLICATED_FOLLOW:
            return "Duplicate invitation";
        case INVIT_DUPLICATE:
            return "Already in the room";
        case INVIT_USER_OFFLINE:
            return "User not connected";
        case INVIT_USER_NOT_IN_ROOM:
            return is_follow
                ? "User not in a room"
                : "We are not in a room";
        case INVIT_EXPIRED:
            return "Expired";
        case INVIT_INVALID_TARGET:
            return "Invalid target";
        case INVIT_MISSION_RESTRICTED:
            return "Mission restricted";
        case INVIT_RANK_RESTRICTED:
            return "Rank restricted";
        case INVIT_FULL_ROOM:
            return "Room is full";
        case INVIT_KICKED:
            return "Kicked from room";
        case INVIT_PRIVATE_ROOM:
            return "Room is private";
        case INVIT_NOT_IN_CLAN:
        case INVIT_NOT_IN_CW:
            return "Room is a clanwar";
        case INVIT_RATING:
            return "Ranked match";
        default:
            return NULL;
    }
}

void invitation_complete(const char *nickname,
                         const char *channel,
                         enum invitation_result r,
                         int is_follow)
{
    if (pending_invitations == NULL)
        return;

    struct invitation *i = list_get(pending_invitations, nickname);

    if (i != NULL)
    {
        if (i->cb != NULL)
            i->cb(channel, r, i->args);

        i->cb = NULL;

        list_remove(pending_invitations, nickname);
    }

    if (r != INVIT_ACCEPTED)
    {
        const char *action = is_follow ? "follow" : "invite";
        const char *reason = _get_invitation_failure(is_follow, r);

        if (reason != NULL)
            eprintf("Failed to %s %s (%s)\n",
                    action, nickname, reason);
        else
            eprintf("Failed to invite %s (%i)\n",
                    nickname, r);
    }
    else if (is_follow == 0)
    {
        xprintf("%s accepted the invitation\n", nickname);
    }
}

static void xmpp_iq_invitation_result_cb(const char *msg_id,
                                         const char *msg,
                                         void *args)
{
    /* Accept any preinvite
       <iq from='xxxx@warface/GameClient' id='uid000000e9' type='get'>
        <query xmlns='urn:cryonline:k01'>
         <invitation_result result='17' user='xxxxxxx' is_follow='0'
                            user_id='xxxxxx'/>
        </query>
       </iq>
     */

    char *data = wf_get_query_content(msg);

    if (!data)
        return;

    char *channel = get_info(msg, "from='masterserver@warface/", "'", NULL);

    char *user = get_info(data, "user='", "'", NULL);
    int is_follow = get_info_int(data, "is_follow='", "'", NULL);
    int result = get_info_int(data, "result='", "'", NULL);

    invitation_complete(user, channel, result, is_follow);

    free(user);
    free(channel);
    free(data);
}

void xmpp_iq_invitation_result_r(void)
{
    qh_register("invitation_result", 1, xmpp_iq_invitation_result_cb, NULL);
}
