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

#ifndef WB_GAMEROOM_H
# define WB_GAMEROOM_H

# include <wb_status.h>
# include <wb_list.h>

enum class
{
    CLASS_RIFLEMAN = 0,
    CLASS_SNIPER = 2,
    CLASS_MEDIC = 3,
    CLASS_ENGINEER = 4
};

enum gameroom_status
{
    GAMEROOM_UNREADY = 0,
    GAMEROOM_READY = 1,
    GAMEROOM_RESTRICTED = 2,
};

#define SYNC_LIST                               \
    X(GR_SYNC_CORE, 0, core)                    \
    X(GR_SYNC_CUSTOM_PARAMS, 1, custom_params)  \
    X(GR_SYNC_MISSION, 2, mission)              \
    X(GR_SYNC_SESSION, 3, session)              \
    X(GR_SYNC_ROOM_MASTER, 4, room_master)      \
    X(GR_SYNC_REGIONS, 5, regions)              \
    X(GR_SYNC_AUTO_START, 6, auto_start)        \
    X(GR_SYNC_CLAN_WAR, 7, clan_war)


enum gr_sync_type
{
#define X(Name, Offset, Field)                  \
    Name = 1 << Offset,

    SYNC_LIST

#undef X
};

typedef struct
{
    unsigned int revision;
    enum gr_sync_type type;
} s_gr_sync;

typedef struct
{
    s_gr_sync base;

    char *room_name;
    char teams_switched;
    char private;
    char can_start;
    char team_balanced;
    unsigned int min_ready_players;
    unsigned int num_players;

    struct list *players;
} s_gr_core;

struct gr_core_player
{
    char *nickname;
    char *clan_name;
    char *profile_id;
    char *online_id;
    char *group_id;
    char *region_id;

    enum class class_id;
    unsigned int team_id;
    enum gameroom_status status;
    enum status presence;

    char observer;
    float skill;
    unsigned int experience;
    unsigned int rank;

    struct {
        unsigned int badge;
        unsigned int mark;
        unsigned int stripe;
    } banner;
};

typedef struct
{
    s_gr_sync base;

    char friendly_fire;
    char enemy_outlines;
    char auto_team_balance;
    char dead_can_chat;
    char join_in_the_process;
    char locked_spectator_camera;

    int preround_time;

    unsigned int max_players;
    unsigned int round_limit;
    unsigned int class_restriction;
    unsigned int inventory_slot;
} s_gr_custom_params;

typedef struct
{
    s_gr_sync base;

    char *master;
} s_gr_room_master;

typedef struct
{
    s_gr_sync base;

    char *id;
    int status;
    int game_progress;

    unsigned start_time;
    unsigned team1_start_score;
    unsigned team2_start_score;
} s_gr_session;

typedef struct
{
    s_gr_sync base;

    char *mission_key;
    char *name;
    char *setting;
    char *mode;
    char *mode_name;
    char *mode_icon;
    char *description;
    char *image;
    char *difficulty;
    char *type;
    char *time_of_day;

    char no_teams;
} s_gr_mission;

typedef struct
{
    s_gr_sync base;

    char *region_id;
} s_gr_regions;

typedef struct
{
    s_gr_sync base;

    int has_timeout;
    int timeout_left;
    int can_manual_start;
    int joined_intermission_timeout;
} s_gr_auto_start;

typedef struct
{
    s_gr_sync base;

    char *clan_1;
    char *clan_2;
} s_gr_clan_war;

enum room_type
{
    ROOM_PVE_PRIVATE = 1,
    ROOM_PVP_PUBLIC = 2,
    ROOM_PVP_CLANWAR = 4,
    ROOM_PVP_QUICKPLAY = 8,
    ROOM_PVE_QUICKPLAY = 16,
    ROOM_PVP_RATING = 32,
};

struct gameroom
{
    char *room_id;
    enum room_type room_type;

    s_gr_core core;
    s_gr_custom_params custom_params;
    s_gr_mission mission;
    s_gr_session session;
    s_gr_room_master room_master;
    s_gr_regions regions;
    s_gr_auto_start auto_start;
    s_gr_clan_war clan_war;
};

void gameroom_init(struct gameroom *gr);
int gameroom_parse(struct gameroom *gr, const char *data);
void gameroom_update(struct gameroom *dst,
                     struct gameroom *src,
                     int changes);
void gameroom_free(struct gameroom *gr);

#endif /* !WB_GAMEROOM_H */
