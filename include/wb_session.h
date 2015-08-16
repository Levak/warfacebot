/**
 * WarfaceBot, a blind XMPP client for Warface (FPS)
 * Copyright (C) 2015 Levak Borok <levak92@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef WB_SESSION_H
# define WB_SESSION_H

# include <wb_list.h>

enum e_status
{
    STATUS_OFFLINE   = 0,
    STATUS_ONLINE    = 1 << 0,
    STATUS_UNK       = 1 << 1,
    STATUS_AFK       = 1 << 2,
    STATUS_LOBBY     = 1 << 3,
    STATUS_ROOM      = 1 << 4,
    STATUS_PLAYING   = 1 << 5,
    STATUS_SHOP      = 1 << 6,
    STATUS_INVENTORY = 1 << 7,
};

enum e_clan_role
{
    CLAN_MASTER = 1,
    CLAN_OFFICER = 2,
    CLAN_MEMBER = 3,
};

enum e_class
{
    CLASS_RIFLEMAN = 0,
    CLASS_SNIPER = 2,
    CLASS_MEDIC = 3,
    CLASS_ENGINEER = 4,
};

struct session
{
    int wfs;
    char active;
    char *jid;
    char *nickname;
    char *active_token;
    char *profile_id;
    char *online_id;
    char *channel;
    char *gameroom_jid;
    struct list *friends;
    struct list *clanmates;
    struct list *missions;
    enum e_status status;
    unsigned int experience;
    unsigned int clan_points;
    enum e_clan_role clan_role;
    unsigned int clan_id;
    char *clan_name;
    unsigned int clan_joined;
    enum e_class curr_class;
    char leaving;
    char ingameroom;
    struct list *rooms;
};

extern struct session session;

void session_init(int fd, const char *online_id);
void session_free(void);

#endif /* WB_SESSION_H */
