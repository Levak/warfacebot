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

#include <wb_gameroom.h>
#include <wb_list.h>
#include <wb_tools.h>
#include <wb_xml.h>

#include <string.h>
#include <stdlib.h>

static int _player_cmp(struct gr_core_player *p, char *profile_id)
{
    return strcmp(p->profile_id, profile_id);
}

static void _player_free(struct gr_core_player *p)
{
    free(p->nickname);
    free(p->clan_name);
    free(p->profile_id);
    free(p->online_id);
    free(p->group_id);
    free(p->region_id);

    free(p);
}

typedef void (* f_sync_func)(void *local, const char *node);

static inline int gameroom_sync_node(s_gr_sync *local,
                                     f_sync_func sync,
                                     const char *node)
{
    if (node == NULL)
        return 0;

    unsigned int revision = get_info_int(node, "revision='", "'", NULL);

    if (revision > local->revision)
    {
        local->revision = revision;
        sync(local, node);

        return local->type;
    }

    return 0;
}

#define SYNC_INT(Local, Node, Name)                      \
    (Local) = get_info_int(Node, Name "='", "'", NULL)

#define SYNC_FLT(Local, Node, Name)                      \
    (Local) = get_info_float(Node, Name "='", "'", NULL)

#define SYNC_STR(Local, Node, Name) do {                 \
        free(Local);                                     \
        (Local) = get_info(Node, Name "='", "'", NULL);  \
    } while (0)                                          \

static void _sync_core(s_gr_core *local, const char *node)
{
    SYNC_INT(local->teams_switched, node, "teams_switched");
    SYNC_INT(local->private, node, "private");
    SYNC_INT(local->can_start, node, "can_start");
    SYNC_INT(local->team_balanced, node, "team_balanced");
    SYNC_INT(local->min_ready_players, node, "min_ready_players");
    SYNC_INT(local->num_players, node, "players");

    {
        const char *r = local->room_name;
        SYNC_STR(local->room_name, node, "room_name");
        if (r != local->room_name)
            xml_deserialize_inplace(&local->room_name);
    }

    char *players = get_info(node, "<players>", "</players>", NULL);

    /* Loop foreach players */
    if (players != NULL)
    {
        const char *m = players;

        list_empty(local->players);

        while ((m = strstr(m, "<player")))
        {
            char *player = get_info(m, "<player", "/>", NULL);

            struct gr_core_player *p =
                calloc(1, sizeof(struct gr_core_player));

            SYNC_STR(p->nickname, player, "nickname");
            SYNC_STR(p->clan_name, player, "clanName");
            SYNC_STR(p->profile_id, player, "profile_id");
            SYNC_STR(p->online_id, player, "online_id");
            SYNC_STR(p->group_id, player, "group_id");
            SYNC_STR(p->region_id, player, "region_id");

            SYNC_INT(p->class_id, player, "class_id");
            SYNC_INT(p->team_id, player, "team_id");
            SYNC_INT(p->status, player, "status");
            SYNC_INT(p->presence, player, "presence");
            SYNC_INT(p->observer, player, "observer");
            SYNC_INT(p->experience, player, "experience");
            SYNC_INT(p->rank, player, "rank");
            SYNC_INT(p->banner.badge, player, "banner_badge");
            SYNC_INT(p->banner.mark, player, "banner_mark");
            SYNC_INT(p->banner.stripe, player, "banner_stripe");

            SYNC_FLT(p->skill, player, "skill");

            list_add(local->players, p);

            free(player);

            ++m;
        }
    }

    free(players);
}

static void _free_core(s_gr_core *local)
{
    if (local->players != NULL)
    {
        list_free(local->players);
        local->players = NULL;
    }

    free(local->room_name);
    local->room_name = NULL;
}

static void _sync_custom_params(s_gr_custom_params *local, const char *node)
{
    SYNC_INT(local->friendly_fire, node, "friendly_fire");
    SYNC_INT(local->enemy_outlines, node, "enemy_outlines");
    SYNC_INT(local->auto_team_balance, node, "auto_team_balance");
    SYNC_INT(local->dead_can_chat, node, "dead_can_chat");
    SYNC_INT(local->join_in_the_process, node, "join_in_the_process");

    SYNC_INT(local->max_players, node, "max_players");
    SYNC_INT(local->round_limit, node, "round_limit");
    SYNC_INT(local->preround_time, node, "preround_time");
    SYNC_INT(local->class_restriction, node, "class_restriction");
    SYNC_INT(local->inventory_slot, node, "inventory_slot");
    SYNC_INT(local->locked_spectator_camera, node, "locked_spectator_camera");
}

static void _free_custom_params(s_gr_custom_params *local)
{

}

static void _sync_regions(s_gr_regions *local, const char *node)
{
    SYNC_STR(local->region_id, node, "regions_id");
}

static void _free_regions(s_gr_regions *local)
{
    free(local->region_id);
    local->region_id = NULL;
}

static void _sync_auto_start(s_gr_auto_start *local, const char *node)
{
    SYNC_INT(local->has_timeout, node, "auto_start_timeout");
    SYNC_INT(local->timeout_left, node, "auto_start_timeout_left");
    SYNC_INT(local->can_manual_start, node, "can_manual_start");
    SYNC_INT(local->joined_intermission_timeout, node, "joined_intermission_timeout");
}

static void _free_auto_start(s_gr_auto_start *local)
{

}

static void _sync_clan_war(s_gr_clan_war *local, const char *node)
{
    SYNC_STR(local->clan_1, node, "clan_1");
    SYNC_STR(local->clan_2, node, "clan_2");
}

static void _free_clan_war(s_gr_clan_war *local)
{
    free(local->clan_1);
    local->clan_1 = NULL;
    free(local->clan_2);
    local->clan_2 = NULL;
}

static void _sync_mission(s_gr_mission *local, const char *node)
{
    SYNC_STR(local->mission_key, node, "mission_key");
    SYNC_STR(local->name, node, "name");
    SYNC_STR(local->setting, node, "setting");
    SYNC_STR(local->mode, node, "mode");
    SYNC_STR(local->mode_name, node, "mode_name");
    SYNC_STR(local->mode_icon, node, "mode_icon");
    SYNC_STR(local->description, node, "description");
    SYNC_STR(local->image, node, "image");
    SYNC_STR(local->difficulty, node, "difficulty");
    SYNC_STR(local->type, node, "type");
    SYNC_STR(local->time_of_day, node, "time_of_day");

    SYNC_INT(local->no_teams, node, "no_teams");
}

static void _free_mission(s_gr_mission *local)
{
    free(local->mission_key);
    local->mission_key = NULL;
    free(local->name);
    local->name = NULL;
    free(local->setting);
    local->setting = NULL;
    free(local->mode);
    local->mode = NULL;
    free(local->mode_name);
    local->mode_name = NULL;
    free(local->mode_icon);
    local->mode_icon = NULL;
    free(local->description);
    local->description = NULL;
    free(local->image);
    local->image = NULL;
    free(local->difficulty);
    local->difficulty = NULL;
    free(local->type);
    local->type = NULL;
    free(local->time_of_day);
    local->time_of_day = NULL;
}

static void _sync_session(s_gr_session *local, const char *node)
{
    SYNC_STR(local->id, node, "id");

    SYNC_INT(local->status, node, "status");
    SYNC_INT(local->game_progress, node, "game_progress");
    SYNC_INT(local->start_time, node, "start_time");
}

static void _free_session(s_gr_session *local)
{
    free(local->id);
    local->id = NULL;
}

static void _sync_room_master(s_gr_room_master *local, const char *node)
{
    SYNC_STR(local->master, node, "master");
}

static void _free_room_master(s_gr_room_master *local)
{
    free(local->master);
    local->master = NULL;
}

void gameroom_init(struct gameroom *gr)
{
    memset(gr, 0, sizeof (struct gameroom));

#define X(Name, Offset, Field)                  \
    gr->Field.base.type = Name;                 \
    gr->Field.base.revision = 0;                \

    SYNC_LIST;

#undef X

    gr->core.players =
        list_new((f_list_cmp) _player_cmp,
                 (f_list_free) _player_free);

}

int gameroom_parse(struct gameroom *gr, const char *data)
{
    int ret = 0;

    gr->room_type =
        get_info_int(data, "room_type='", "'", NULL);

    free(gr->room_id);
    gr->room_id =
        get_info(data, "room_id='", "'", NULL);

#define X(Name, Offset, Field) {                                        \
        char *n = NULL;                                                 \
        n = get_info(data, "<" #Field, "</" #Field ">", NULL);          \
        n = n ? n : get_info(data, "<" #Field " ", "/>", NULL);         \
        ret |= gameroom_sync_node((s_gr_sync *) &gr->Field,             \
                                  (f_sync_func) _sync_ ## Field,        \
                                  n);                                   \
        free(n);                                                        \
    }

    SYNC_LIST;

#undef X

    return ret;
}

void gameroom_update(struct gameroom *dst,
                     struct gameroom *src,
                     int changes)
{
    free(dst->room_id);
    dst->room_id = src->room_id;
    src->room_id = NULL;

    dst->room_type = src->room_type;

#define UPDATE(Dst, Src, Changes, Filter, Field) do {                    \
        if ((Changes) & (Filter))                                        \
        {                                                                \
            _free_ ## Field (&(Dst)->Field);                             \
            memcpy(&(Dst)->Field, &(Src)->Field, sizeof ((Src)->Field)); \
        }                                                                \
    } while (0)

#define X(Name, Offset, Field)                  \
    UPDATE(dst, src, changes, Name, Field);

    SYNC_LIST;

#undef X

#undef UPDATE
}

void gameroom_free(struct gameroom *gr)
{
    free(gr->room_id);
    gr->room_id = NULL;

#define X(Name, Offset, Field)                  \
    _free_ ## Field (&gr->Field);               \
    gr->Field.base.revision = 0;

    SYNC_LIST;

#undef X
}
