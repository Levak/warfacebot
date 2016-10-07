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

#ifndef WB_CVAR_H
# define WB_CVAR_H

#include <stdlib.h>

# define CVAR_LIST_                             \
    XSTR(game_version, NULL)                    \
    XSTR(game_server_name, NULL)                \
    XINT(game_hwid, 0)                          \
    XINT(game_max_unlocked_items, 111)          \
                                                \
    XSTR(online_resource, "GameClient")         \
    XSTR(online_region_id, "global")            \
    XSTR(online_server, NULL)                   \
    XSTR(online_host, "warface") /* todo */     \
    XINT(online_server_port, 5222)              \
    XSTR(online_channel_type, "pve")            \
    XINT(online_pvp_rank, 13)                   \
    XBOOL(online_use_protect, 1)                \
    XBOOL(online_use_tls, 1)                    \
                                                \
    XBOOL(wb_safemaster, 0)                     \
    XSTR(wb_safemaster_room_name, "FairGame (rank > 15 & kdr < 3)")\
    XSTR(wb_safemaster_channel, "pvp_pro_1")    \
    XBOOL(wb_accept_friend_requests, 1)         \
    XBOOL(wb_postpone_friend_requests, 0)       \
    XBOOL(wb_accept_clan_invites, 1)            \
    XBOOL(wb_postpone_clan_invites, 0)          \
    XBOOL(wb_enable_whisper_commands, 1)        \
    XBOOL(wb_leave_on_start, 1)                 \
    XBOOL(wb_accept_room_invitations, 1)        \
    XBOOL(wb_enable_invite, 1)                  \
    XINT(wb_ping_unit, 60)                      \
    XINT(wb_ping_count_is_stall, 3)             \
    XINT(wb_ping_count_is_over, 4)              \
    XINT(wb_ping_count_is_outdated, 5)          \
                                                \
    XSTR(query_cache_location, "./QueryCache/") \
    XBOOL(query_cache, 1)                       \
    XBOOL(query_disable_items, 0)               \
    XBOOL(query_disable_shop_get_offers, 0)     \
    XBOOL(query_disable_quickplay_maplist, 0)   \
    XBOOL(query_disable_get_configs, 0)         \

# ifdef DEBUG
#  define CVAR_LIST                             \
    CVAR_LIST_                                  \
    XBOOL(query_debug, 1)                       \

#else /* DEBUG */
#  define CVAR_LIST CVAR_LIST_
#endif /* DEBUG */

typedef int t_cvar_int;
typedef char *t_cvar_str;
typedef char t_cvar_bool;

char *cvar_copy(const char *val);

# define CVAR_STR_SET(Name, Value) do {                 \
        t_cvar_str *p = (t_cvar_str *) &cvar.Name;      \
        free(*p), *p = cvar_copy(Value);                \
    } while (0)

struct cvar
{
# define XINT(Name, DefaultValue) t_cvar_int Name;
# define XSTR(Name, DefaultValue) const t_cvar_str Name;
# define XBOOL(Name, DefaultValue) t_cvar_bool Name;
    CVAR_LIST
# undef XINT
# undef XSTR
# undef XBOOL
};

extern struct cvar cvar;

void cvar_init(void);
void cvar_free(void);

enum e_cvar_parse
{
    CVAR_PARSE_SUCCESS,
    CVAR_PARSE_NOTFOUND,
    CVAR_PARSE_ERROR,
};

int cvar_set(const char *name, const char *value);
enum e_cvar_parse cvar_parse_file(const char *path);

#endif /* WB_CVAR_H */
