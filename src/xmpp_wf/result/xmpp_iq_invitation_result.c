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
#include <wb_log.h>
#include <wb_lang.h>

#include <stdlib.h>

static struct list *pending_invitations = NULL;

struct invitation
{
    char *nickname;
    char is_follow;
    f_invitation_result_cb cb;
    void *args;
};

static int invitation_cmp_follow(struct invitation *i, const char *unused)
{
    return i->is_follow == 1 ? 0 : 1;
}

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
                         char is_follow,
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

    struct invitation *old_invit;

    if (is_follow)
        old_invit = list_get_by(pending_invitations,
                                NULL,
                                (f_list_cmp) invitation_cmp_follow);
    else
        old_invit = list_get(pending_invitations, nickname);

    if (old_invit != NULL)
    {
        return;
    }

    struct invitation *new_invit = calloc(1, sizeof (struct invitation));

    new_invit->nickname = strdup(nickname);
    new_invit->is_follow = is_follow;
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
            return LANG(invite_rejected);
        case INVIT_PENDING:
            return LANG(invite_pending);
        case INVIT_AUTOREJECT:
            return LANG(invite_autoreject);
        case INVIT_DUPLICATED_FOLLOW:
            return LANG(invite_duplicate_follow);
        case INVIT_DUPLICATE:
            return LANG(invite_duplicate);
        case INVIT_USER_OFFLINE:
            return LANG(invite_user_offline);
        case INVIT_USER_NOT_IN_ROOM:
            return is_follow
                ? LANG(invite_user_not_in_room)
                : LANG(invite_not_in_room);
        case INVIT_EXPIRED:
            return LANG(invite_expired);
        case INVIT_INVALID_TARGET:
            return LANG(invite_invalid_target);
        case INVIT_MISSION_RESTRICTED:
            return LANG(invite_mission_restricted);
        case INVIT_RANK_RESTRICTED:
            return LANG(invite_rank_restricted);
        case INVIT_FULL_ROOM:
            return LANG(invite_full_room);
        case INVIT_KICKED:
            return LANG(invite_kicked);
        case INVIT_PRIVATE_ROOM:
            return LANG(invite_private_room);
        case INVIT_NOT_IN_CLAN:
        case INVIT_NOT_IN_CW:
            return LANG(invite_not_in_cw);
        case INVIT_RATING:
            return LANG(invite_rating);
        default:
            return NULL;
    }
}

int invitation_complete(const char *nickname,
                         const char *channel,
                         enum invitation_result r,
                         int is_follow)
{
    if (pending_invitations == NULL)
        return 0;

    struct invitation *i;

    if (is_follow)
        i = list_get_by(pending_invitations,
                        NULL,
                        (f_list_cmp) invitation_cmp_follow);
    else
        i = list_get(pending_invitations, nickname);

    if (r != INVIT_ACCEPTED)
    {
        const char *reason = _get_invitation_failure(is_follow, r);
        const char *l_nick = i != NULL ? i->nickname : nickname;

        char *s = (is_follow)
            ? LANG_FMT(error_follow, l_nick)
            : LANG_FMT(error_invite, l_nick);

        if (reason != NULL)
            eprintf("%s (%s)", s, reason);
        else
            eprintf("%s (%i)", s, r);

        free(s);
    }
    else
    {
        char *s = (is_follow)
            ? LANG_FMT(follow_accepted, nickname)
            : LANG_FMT(invite_accepted, nickname);

        xprintf("%s", s);
        free(s);
    }

    if (i != NULL)
    {
        if (i->cb != NULL)
            i->cb(channel, r, i->args);

        i->cb = NULL;

        list_remove(pending_invitations, nickname);
        i = NULL;

        return 1;
    }
    else
    {
        return 0;
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
