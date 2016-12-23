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

#include <wb_gameroom.h>
#include <wb_list.h>
#include <wb_tools.h>

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

    SYNC_STR(local->room_name, node, "room_name");

    char *players = get_info(node, "<players>", "</players>", NULL);

    /* Loop foreach players */
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

static void _sync_regions(s_gr_regions *local, const char *node)
{
    SYNC_STR(local->region_id, node, "regions_id");
}

static void _sync_auto_start(s_gr_auto_start *local, const char *node)
{
    SYNC_INT(local->has_timeout, node, "auto_start_timeout");
    SYNC_INT(local->timeout_left, node, "auto_start_timeout_left");
    SYNC_INT(local->can_manual_start, node, "can_manual_start");
    SYNC_INT(local->joined_intermission_timeout, node, "joined_intermission_timeout");
}

static void _sync_clan_war(s_gr_clan_war *local, const char *node)
{
    SYNC_STR(local->clan_1, node, "clan_1");
    SYNC_STR(local->clan_2, node, "clan_2");
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

static void _sync_session(s_gr_session *local, const char *node)
{
    SYNC_STR(local->id, node, "id");

    SYNC_INT(local->status, node, "status");
    SYNC_INT(local->game_progress, node, "game_progress");
    SYNC_INT(local->start_time, node, "start_time");
}

static void _sync_room_master(s_gr_room_master *local, const char *node)
{
    SYNC_STR(local->master, node, "master");
}

void gameroom_init(struct gameroom *gr)
{
    gr->core.players =
        list_new((f_list_cmp) _player_cmp,
                 (f_list_free) _player_free);

    gr->core.base.type = GR_SYNC_CORE;
    gr->core.base.revision = 0;

    gr->custom_params.base.type = GR_SYNC_CUSTOM_PARAMS;
    gr->custom_params.base.revision = 0;

    gr->mission.base.type = GR_SYNC_MISSION;
    gr->mission.base.revision = 0;

    gr->session.base.type = GR_SYNC_SESSION;
    gr->session.base.revision = 0;

    gr->room_master.base.type = GR_SYNC_ROOM_MASTER;
    gr->room_master.base.revision = 0;

    gr->auto_start.base.type = GR_SYNC_AUTO_START;
    gr->auto_start.base.revision = 0;

    gr->regions.base.type = GR_SYNC_REGIONS;
    gr->regions.base.revision = 0;

    gr->clan_war.base.type = GR_SYNC_CLAN_WAR;
    gr->clan_war.base.revision = 0;
}

int gameroom_parse(struct gameroom *gr, const char *data)
{
    int ret = 0;

    char *core_node = get_info(data, "<core ", "</core>", NULL);
    if (core_node == NULL)
        core_node = get_info(data, "<core ", "/>", NULL);

    char *session_node = get_info(data, "<session ", "/>", NULL);
    char *mission_node = get_info(data, "<mission ", ">", NULL);
    char *room_master_node = get_info(data, "<room_master ", "/>", NULL);
    char *custom_params_node = get_info(data, "<custom_params ", "/>", NULL);
    char *regions_node = get_info(data, "<regions ", "/>", NULL);
    char *auto_start_node = get_info(data, "<auto_start ", "/>", NULL);
    char *clan_war_node = get_info(data, "<clan_war ", "/>", NULL);

    gr->room_type =
        get_info_int(data, "room_type='", "'", NULL);

    free(gr->room_id);
    gr->room_id =
        get_info(data, "room_id='", "'", NULL);

    ret |= gameroom_sync_node((s_gr_sync *) &gr->core,
                              (f_sync_func) _sync_core,
                              core_node);

    ret |= gameroom_sync_node((s_gr_sync *) &gr->custom_params,
                              (f_sync_func) _sync_custom_params,
                              custom_params_node);

    ret |= gameroom_sync_node((s_gr_sync *) &gr->mission,
                              (f_sync_func) _sync_mission,
                              mission_node);

    ret |= gameroom_sync_node((s_gr_sync *) &gr->session,
                              (f_sync_func) _sync_session,
                              session_node);

    ret |= gameroom_sync_node((s_gr_sync *) &gr->room_master,
                              (f_sync_func) _sync_room_master,
                              room_master_node);

    ret |= gameroom_sync_node((s_gr_sync *) &gr->regions,
                              (f_sync_func) _sync_regions,
                              regions_node);

    ret |= gameroom_sync_node((s_gr_sync *) &gr->auto_start,
                              (f_sync_func) _sync_auto_start,
                              auto_start_node);

    ret |= gameroom_sync_node((s_gr_sync *) &gr->clan_war,
                              (f_sync_func) _sync_clan_war,
                              clan_war_node);

    free(core_node);
    free(session_node);
    free(mission_node);
    free(room_master_node);
    free(custom_params_node);
    free(regions_node);
    free(auto_start_node);
    free(clan_war_node);

    return ret;
}

void gameroom_free(struct gameroom *gr)
{
    if (gr->core.players != NULL)
    {
        list_free(gr->core.players);
        gr->core.players = NULL;
    }

    free(gr->room_id);
    gr->room_id = NULL;

    free(gr->core.room_name);
    gr->core.room_name = NULL;

    free(gr->mission.mission_key);
    gr->mission.mission_key = NULL;
    free(gr->mission.name);
    gr->mission.name = NULL;
    free(gr->mission.setting);
    gr->mission.setting = NULL;
    free(gr->mission.mode);
    gr->mission.mode = NULL;
    free(gr->mission.mode_name);
    gr->mission.mode_name = NULL;
    free(gr->mission.mode_icon);
    gr->mission.mode_icon = NULL;
    free(gr->mission.description);
    gr->mission.description = NULL;
    free(gr->mission.image);
    gr->mission.image = NULL;
    free(gr->mission.difficulty);
    gr->mission.difficulty = NULL;
    free(gr->mission.type);
    gr->mission.type = NULL;
    free(gr->mission.time_of_day);
    gr->mission.time_of_day = NULL;

    free(gr->session.id);
    gr->session.id = NULL;

    free(gr->room_master.master);
    gr->room_master.master = NULL;

    free(gr->regions.region_id);
    gr->regions.region_id = NULL;

    free(gr->clan_war.clan_1);
    gr->clan_war.clan_1 = NULL;
    free(gr->clan_war.clan_2);
    gr->clan_war.clan_2 = NULL;

    /* Reset revisions */
    gr->core.base.revision = 0;
    gr->custom_params.base.revision = 0;
    gr->mission.base.revision = 0;
    gr->session.base.revision = 0;
    gr->room_master.base.revision = 0;
    gr->auto_start.base.revision = 0;
    gr->regions.base.revision = 0;
    gr->clan_war.base.revision = 0;
}
