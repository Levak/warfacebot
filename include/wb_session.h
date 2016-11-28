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

#ifndef WB_SESSION_H
# define WB_SESSION_H

# include <wb_list.h>
# include <wb_querycache.h>
# include <wb_status.h>

# include <time.h>

enum e_clan_role
{
    CLAN_MASTER = 1,
    CLAN_OFFICER = 2,
    CLAN_MEMBER = 3
};

enum e_class
{
    CLASS_RIFLEMAN = 0,
    CLASS_SNIPER = 2,
    CLASS_MEDIC = 3,
    CLASS_ENGINEER = 4
};

enum e_gameroom_status
{
    GAMEROOM_UNREADY = 0,
    GAMEROOM_READY = 1,
    GAMEROOM_RESTRICTED = 2,
};

enum gr_sync_type
{
    GR_SYNC_CORE          = 1 << 0,
    GR_SYNC_CUSTOM_PARAMS = 1 << 1,
    GR_SYNC_MISSION       = 1 << 2,
    GR_SYNC_SESSION       = 1 << 3,
    GR_SYNC_ROOM_MASTER   = 1 << 4,
    GR_SYNC_REGIONS       = 1 << 5,
    GR_SYNC_AUTO_START    = 1 << 6,
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

    enum e_class class_id;
    unsigned int team_id;
    enum e_gameroom_status status;
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
    int start_time;
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
    char *online_id;
    char *profile_id;
    char *nickname;
    char accepted;
} s_qp_player;

enum session_state
{
    STATE_DEAD = 0,
    STATE_INIT,
    STATE_POLL,
    STATE_TLS_INIT,
    STATE_RUN,
};

struct session
{
    int wfs;
    enum session_state state;

    struct {
        char *jid;
        time_t last_query;
        struct list *rooms;
    } xmpp;

    struct {
        char *id;
        char *active_token;
        char *channel;
        char *channel_type;
        enum status status;
        time_t last_status_update;

        char *place_token;
        char *place_info_token;
        char *mode_info_token;
        char *mission_info_token;

        struct list *masterservers;
    } online;

    struct {
        struct {
            struct list *list;
            int hash;
            int content_hash;
        } missions;

        QUERYCACHE(struct list *, offers) shop;

        QUERYCACHE(struct list *, list) items;

        QUERYCACHE(struct game_config *, game) config;
    } wf;

    struct {
        char *pre_uid;
        char *uid;
        struct list *group;

        char *mission_id;
        char *game_mode;
        int type;
        int channel_switches;

        QUERYCACHE(struct list *, list) maps;
    } quickplay;

    struct {
        char *jid;
        char *room_id;
        char leaving;
        char joined;

        char *group_id;
        int curr_team;
        int curr_status;
        int desired_status;

        time_t leave_timeout;

        struct {
            s_gr_core core;
            s_gr_custom_params custom_params;
            s_gr_mission mission;
            s_gr_session session;
            s_gr_room_master room_master;
            s_gr_regions regions;
            s_gr_auto_start auto_start;
        } sync;
    } gameroom;

    struct {
        char *id;
        char *nickname;

        unsigned int experience;
        enum e_class curr_class;

        struct list *items;

        struct list *friends;
        struct list *clanmates;

        struct {
            unsigned int game;
            unsigned int crown;
            unsigned int cry;
        } money;

        char *primary_weapon;

        struct {
            unsigned int badge;
            unsigned int mark;
            unsigned int stripe;
        } banner;

        struct {
            unsigned int items_unlocked;
            unsigned int challenges_completed;
            unsigned int playtime_seconds;
            float leavings_percentage;
            unsigned int coop_climbs_performed;
            unsigned int coop_assists_performed;

            struct {
                unsigned int rating_points;
                unsigned int wins;
                unsigned int loses;
                unsigned int kills;
                unsigned int deaths;
                unsigned int favorite_class; /* TODO */
            } pvp;

            struct {
                unsigned int missions_completed;
                unsigned int favorite_class; /* TODO */
            } pve;
        } stats;

        struct {
            unsigned int id;
            unsigned int own_position;
            unsigned int points;
            unsigned int joined;
            char *name;
            enum e_clan_role role;

            struct {
                unsigned int badge;
                unsigned int mark;
                unsigned int stripe;
            } master;
        } clan;
    } profile;
};

extern struct session session;

void session_init(int fd);
void session_free(void);

#endif /* WB_SESSION_H */
