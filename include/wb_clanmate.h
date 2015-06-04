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

#ifndef WB_CLANMATE_H
# define WB_CLANMATE_H

# include <wb_session.h>

struct clanmate
{
    char *jid;
    char *nickname;
    char *profile_id;
    enum e_status status;
    unsigned int experience;
    unsigned int clan_points;
    enum e_clan_role clan_role;
};

void clanmate_list_add(const char *jid,
                       const char *nickname,
                       const char *profile_id,
                       int status,
                       int experience,
                       int clan_points,
                       int clan_role);

void clanmate_list_update(const char *jid,
                          const char *nickname,
                          const char *profile_id,
                          int status,
                          int experience,
                          int clan_points,
                          int clan_role);

void clanmate_list_remove(const char *nickname);
void clanmate_list_empty(void);
void clanmate_list_init(void);
void clanmate_list_free(void);

#endif /* !WB_CLANMATE_H */
