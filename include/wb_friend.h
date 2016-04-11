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

#ifndef WB_FRIEND_H
# define WB_FRIEND_H

# include <wb_session.h>

struct friend
{
    char *jid;
    char *nickname;
    char *profile_id;
    enum e_status status;
    unsigned int experience;
	char *place_token;
	char *place_info_token;
	char *mode_info_token;
	char *mission_info_token;
};

void friend_list_add(const char *jid,
                     const char *nickname,
                     const char *profile_id,
                     int status,
                     int experience);

void friend_list_update(const char *jid,
                        const char *nickname,
                        const char *profile_id,
                        int status,
                        int experience,
						char *place_token,
						char *place_info_token,
						char *mode_info_token,
						char *mission_info_token);

void friend_list_remove(const char *nickname);
void friend_list_empty(void);
void friend_list_init(void);
void friend_list_free(void);

#endif /* !WB_FRIEND_H */
