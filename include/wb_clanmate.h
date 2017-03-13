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

#ifndef WB_CLANMATE_H
# define WB_CLANMATE_H

# include <wb_session.h>

struct clanmate
{
    char *jid;
    char *nickname;
    char *profile_id;
    enum status status;
    unsigned int experience;
    unsigned int clan_points;
    enum e_clan_role clan_role;
};

enum clan_update
{
    CLAN_UPDATE_CHANGED,
    CLAN_UPDATE_JOINED,
    CLAN_UPDATE_LEFT,
};

void clanmate_free(struct clanmate *f);
struct clanmate *clanmate_new(const char *jid,
                              const char *nickname,
                              const char *profile_id,
                              int status,
                              int experience,
                              int clan_points,
                              int clan_role);

/**
 * @brief Add/Update/Remove a player from the locally saved clan list
 *
 * @param jid         Player Jabber ID. If NULL, means the player is offline
 * @param nickname    Player nickname. If NULL, means the player left the clan
 * @param profile_id  Player Profile ID. Primary key, cannot be NULL
 * @param status
 * @param experience
 * @param clan_points
 * @param clan_role
 */
enum clan_update clanmate_list_update(const char *jid,
                                      const char *nickname,
                                      const char *profile_id,
                                      int status,
                                      int experience,
                                      int clan_points,
                                      int clan_role);

void clanmate_list_empty(void);
void clanmate_list_init(void);
void clanmate_list_free(void);

struct clanmate *clanmate_list_get(const char *nick);
struct clanmate *clanmate_list_get_by_pid(const char *pid);

#endif /* !WB_CLANMATE_H */
