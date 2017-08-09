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

#ifndef WB_SESSION_H
# define WB_SESSION_H

# include <wb_list.h>
# include <wb_querycache.h>
# include <wb_gameroom.h>
# include <wb_status.h>

# include <time.h>
# include <stdio.h>

enum e_clan_role
{
    CLAN_MASTER = 1,
    CLAN_OFFICER = 2,
    CLAN_MEMBER = 3
};

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
    FILE *log_file;

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
        time_t last_status_change;

        char *place_token;
        char *place_info_token;
        char *mode_info_token;
        char *mission_info_token;

        struct list *masterservers;

        struct {
            char *k01;
            char *ms;
            char *muc;
            char *host;
        } jid;

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

        unsigned int total_locked_items;
    } wf;

    struct {
        char *pre_uid;
        char *uid;
        int try_again;
        struct list *group;

        char *mission_id;
        char *game_mode;
        int type;
        int channel_switches;

        QUERYCACHE(struct list *, list) maps;
    } quickplay;

    struct {
        char *jid;
        char leaving;
        char joined;

        char *group_id;
        int curr_team;
        int curr_status;
        int desired_status;

        time_t leave_timeout;

        struct gameroom sync;
    } gameroom;

    struct {
        char *id;
        char *nickname;

        unsigned int experience;
        enum class curr_class;

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

void session_init(int fd, const char *online_id);
void session_free(void);

#endif /* WB_SESSION_H */
