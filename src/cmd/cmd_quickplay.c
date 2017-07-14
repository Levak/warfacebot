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
#include <wb_mission.h>
#include <wb_quickplay.h>
#include <wb_xmpp_wf.h>
#include <wb_tools.h>
#include <wb_log.h>
#include <wb_cmd.h>
#include <wb_lang.h>

static void _cmd_quickplay_invite_cb(const char *info, void *args)
{
    if (info == NULL)
    {
        eprintf("%s", LANG(error_no_user));
        return;
    }

    char *online_id = get_info(info, "online_id='", "'", NULL);
    char *profile_id = get_info(info, "profile_id='", "'", NULL);
    char *nickname = get_info(info, "nickname='", "'", NULL);

    if (online_id != NULL && profile_id != NULL && nickname != NULL)
    {
        quickplay_preinvite(online_id, profile_id, nickname);
    }
    else
    {
        eprintf("%s", LANG(error_no_user));
    }

    free(online_id);
    free(profile_id);
    free(nickname);
}

static void _cmd_quickplay_invite(const char *nickname)
{
    if (nickname == NULL)
        return;

    struct friend *f = friend_list_get(nickname);
    struct clanmate *c = clanmate_list_get(nickname);

    if (f != NULL)
    {
        if (f->jid != NULL)
            quickplay_preinvite(f->jid, f->profile_id, f->nickname);
        else
            eprintf("%s", LANG(error_user_not_connected));
    }
    else if (c != NULL)
    {
        if (c->jid != NULL)
            quickplay_preinvite(c->jid, c->profile_id, c->nickname);
        else
            eprintf("%s", LANG(error_user_not_connected));
    }
    else
    {
        xmpp_iq_profile_info_get_status(nickname,
                                        _cmd_quickplay_invite_cb,
                                        NULL);
    }
}

static void _cmd_quickplay_open(const char *mission_name)
{
    if (0 == strcmp(mission_name, "pvp"))
    {
        quickplay_open(NULL,
                       ROOM_PVP_QUICKPLAY,
                       NULL,
                       NULL,
                       NULL);
    }
    else if (0 == strcmp(mission_name, "rating"))
    {
        quickplay_open(mission_name,
                       ROOM_PVP_RATING,
                       "ptb",
                       NULL,
                       NULL);
    }
    else if ((0 == strcmp(mission_name, "tdm"))
             || (0 == strcmp(mission_name, "ptb"))
             || (0 == strcmp(mission_name, "stm"))
             || (0 == strcmp(mission_name, "dmn"))
             || (0 == strcmp(mission_name, "ffa"))
             || (0 == strcmp(mission_name, "hnt"))
             || (0 == strcmp(mission_name, "ctf"))
             || (0 == strcmp(mission_name, "dst"))
             || (0 == strcmp(mission_name, "tbs")))
    {
        quickplay_open(NULL,
                       ROOM_PVP_QUICKPLAY,
                       mission_name,
                       NULL,
                       NULL);
    }
    else
    {
        struct mission *m = mission_list_get(mission_name);

        if (m != NULL)
        {
            quickplay_open(m->mission_key,
                           strstr(m->mode, "pvp") != NULL
                           ? ROOM_PVP_QUICKPLAY
                           : ROOM_PVE_QUICKPLAY,
                           NULL,
                           NULL,
                           NULL);
        }
        else
        {
            eprintf("%s: '%s'",
                    LANG(error_no_map),
                    mission_name);
        }
    }
}

void cmd_quickplay(const char *cmd, const char *arg_1)
{
    if (cmd == NULL)
        return;

    if (0 == strcmp(cmd, "open"))
    {
        if (arg_1 == NULL)
        {
            eprintf("%s", LANG(console_quickplay_map_required));
            return;
        }

        _cmd_quickplay_open(arg_1);
    }
    else if (0 == strcmp(cmd, "cancel"))
    {
        quickplay_cancel();
    }
    else if (0 == strcmp(cmd, "invite"))
    {
        if (arg_1 == NULL)
        {
            eprintf("%s", LANG(console_quickplay_nick_required));
            return;
        }

        _cmd_quickplay_invite(arg_1);
    }
    else if (0 == strcmp(cmd, "start"))
    {
        quickplay_start(NULL, NULL);
    }
}

void cmd_quickplay_wrapper(const char *cmd, const char *arg_1)
{
    cmd_quickplay(cmd, arg_1);
}

int cmd_quickplay_completions(struct list *l, int arg_index)
{
    switch (arg_index)
    {
        case 1:
            list_add(l, strdup("open"));
            list_add(l, strdup("invite"));
            list_add(l, strdup("cancel"));
            list_add(l, strdup("start"));
            break;
        case 2:
            /* Arg for "invite" */
            complete_buddies(l);

            /* Arg for "open" */
            list_add(l, strdup("rating"));
            list_add(l, strdup("pvp"));

            list_add(l, strdup("tdm"));
            list_add(l, strdup("ptb"));
            list_add(l, strdup("stm"));
            list_add(l, strdup("dmn"));
            list_add(l, strdup("ffa"));
            list_add(l, strdup("ctf"));
            list_add(l, strdup("hnt"));
            list_add(l, strdup("tbs"));
            list_add(l, strdup("dst"));

            complete_missions_pve(l);
            complete_quickplay_maps_pvp(l);
            break;
        default:
            break;
    }

    return 1;
}
