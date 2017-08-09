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
#include <wb_list.h>
#include <wb_mission.h>
#include <wb_xmpp_wf.h>
#include <wb_log.h>
#include <wb_masterserver.h>
#include <wb_quickplay.h>
#include <wb_lang.h>

static int qp_player_cmp(s_qp_player *p, const char *online_id)
{
    return strcmp(p->online_id, online_id);
}

static void qp_player_free(s_qp_player *p)
{
    free(p->online_id);
    free(p->profile_id);
    free(p->nickname);
    free(p);
}

static void _quickplay_cancel_cb(void *args)
{
    xprintf("%s", LANG(quickplay_canceled));
}

static void _quickplay_cancel_preinvite(s_qp_player *p, void *args)
{
    xmpp_iq_preinvite_cancel(
        p->online_id,
        session.quickplay.pre_uid,
        PREINVITE_CANCELED_BY_MASTER,
        NULL, NULL);
}

void quickplay_cancel(void)
{
    if (session.quickplay.pre_uid != NULL
        && session.quickplay.group != NULL)
    {
        list_foreach(session.quickplay.group,
                     (f_list_callback) _quickplay_cancel_preinvite,
                     NULL);
    }

    if (session.quickplay.uid != NULL)
    {
        xmpp_iq_gameroom_quickplay_cancel(_quickplay_cancel_cb, NULL);
    }

    quickplay_free();
}

struct cb_args
{
    f_gameroom_quickplay_cb cb;
    void *args;
};

void quickplay_start(f_gameroom_quickplay_cb cb,
                     void *args)
{
    if (session.quickplay.pre_uid == NULL)
    {
        eprintf("%s", LANG(quickplay_no_pending));
        return;
    }

    xmpp_iq_gameroom_quickplay(
        session.quickplay.pre_uid,
        session.quickplay.mission_id,
        session.quickplay.type,
        session.quickplay.game_mode,
        session.quickplay.channel_switches,
        cb, args);
}

void quickplay_open(const char *mission_key,
                    enum room_type type,
                    const char *game_mode,
                    f_join_channel_cb cb,
                    void *args)
{
    if (session.quickplay.uid != NULL)
    {
        eprintf("%s", LANG(quickplay_in_progress));
        return;
    }

    /* Generate new UID */
    session.quickplay.pre_uid = new_random_uuid();

    session.quickplay.type = type;
    session.quickplay.channel_switches = 0;

    if (mission_key != NULL)
        session.quickplay.mission_id = strdup(mission_key);
    if (game_mode != NULL)
        session.quickplay.game_mode = strdup(game_mode);

    int is_pve = (type & ROOM_PVE_QUICKPLAY) != 0;
    int were_in_pve =
        strstr(session.online.channel_type, "pve") != NULL;

    if (is_pve == were_in_pve)
    {
        if (cb)
            cb(args);
    }
    else
    {
        const char *ms_type = (is_pve) ? "pve" : "pvp_pro";

        struct masterserver *ms =
            masterserver_list_get_by_type(ms_type);

        if (ms != NULL)
        {
            xmpp_iq_join_channel(ms->resource, cb, args);
        }
        else
        {
            char *s = LANG_FMT(error_no_channel_type,
                               ms_type);
            eprintf("%s", s);
            free(s);
        }
    }
}

void quickplay_preinvite(const char *online_id,
                         const char *profile_id,
                         const char *nickname)
{
    if (online_id == NULL || profile_id == NULL || nickname == NULL)
        return;

    if (session.quickplay.pre_uid == NULL)
    {
        eprintf("%s", LANG(quickplay_no_pending));
        return;
    }

    if (session.quickplay.group == NULL)
    {
        session.quickplay.group = list_new(
            (f_list_cmp) qp_player_cmp,
            (f_list_free) qp_player_free);
    }

    s_qp_player *p = calloc(1, sizeof (s_qp_player));

    p->online_id = strdup(online_id);
    p->profile_id = strdup(profile_id);
    p->nickname = strdup(nickname);
    p->accepted = 0;

    list_add(session.quickplay.group, p);

    xmpp_iq_preinvite_invite(online_id,
                             session.quickplay.pre_uid,
                             NULL, NULL);
}

void quickplay_preinvite_response(const char *uid,
                                  const char *online_id,
                                  int accepted)
{
    if (online_id == NULL
        || session.quickplay.group == NULL)
        return;

    s_qp_player *p = list_get(session.quickplay.group, online_id);

    if (p != NULL
        && uid != NULL
        && session.quickplay.pre_uid != NULL
        && 0 == strcmp(uid, session.quickplay.pre_uid))
    {
        {
            char *s =
                (accepted)
                ? LANG_FMT(preinvite_accepted, p->nickname)
                : (p->accepted)
                ? LANG_FMT(preinvite_canceled, p->nickname)
                : LANG_FMT(preinvite_rejected, p->nickname);

            xprintf("%s", s);
            free(s);
        }

        if (accepted)
        {
            p->accepted = 1;
        }
        else
        {
            list_remove(session.quickplay.group, online_id);
        }
    }
    else
    {
        xmpp_iq_preinvite_cancel(
            online_id,
            uid,
            PREINVITE_EXPIRED,
            NULL, NULL);
    }
}

void quickplay_started(const char *uid)
{
    if (uid != NULL
        && session.quickplay.pre_uid != NULL
        && 0 == strcmp(uid, session.quickplay.pre_uid))
    {
        /* If UUID is valid, set .uid and reset .pre_uid */
        free(session.quickplay.uid);
        session.quickplay.uid = strdup(uid);

        free(session.quickplay.pre_uid);
        session.quickplay.pre_uid = NULL;

        session.quickplay.try_again = 0;

        xprintf("%s", LANG(quickplay_started));
    }
}

void quickplay_succeeded(const char *uid)
{
    if (session.quickplay.try_again != 0)
    {
        session.quickplay.try_again = 0;
    }
    else if (uid != NULL
        && session.quickplay.uid != NULL
        && 0 == strcmp(uid, session.quickplay.uid))
    {
        quickplay_free();

        xprintf("%s", LANG(quickplay_done));
    }
}

void quickplay_canceled(const char *uid)
{
    if (uid != NULL
        && session.quickplay.uid != NULL
        && 0 == strcmp(uid, session.quickplay.uid))
    {
        quickplay_free();

        xprintf("%s", LANG(quickplay_canceled));
    }
}

void quickplay_init(void)
{
}

void quickplay_free(void)
{
    free(session.quickplay.pre_uid);
    session.quickplay.pre_uid = NULL;
    free(session.quickplay.uid);
    session.quickplay.uid = NULL;
    free(session.quickplay.mission_id);
    session.quickplay.mission_id = NULL;
    free(session.quickplay.game_mode);
    session.quickplay.game_mode = NULL;

    if (session.quickplay.group != NULL)
        list_free(session.quickplay.group);
    session.quickplay.group = NULL;
}
