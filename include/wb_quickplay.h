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

#ifndef WB_QUICKPLAY_H
# define WB_QUICKPLAY_H

# include <wb_xmpp_wf.h>
# include <wb_session.h>

struct quickplay_map
{
    char *mission;
};

void quickplay_init(void);

void quickplay_open(const char *mission_key,
                    enum room_type type,
                    const char *game_mode,
                    f_join_channel_cb cb,
                    void *args);

void quickplay_start(f_gameroom_quickplay_cb cb,
                     void *args);

void quickplay_preinvite(const char *online_id,
                         const char *profile_id,
                         const char *nickname);

void quickplay_preinvite_response(const char *uid,
                                  const char *online_id,
                                  int accepted);

void quickplay_cancel(void);

void quickplay_started(const char *uid);

void quickplay_canceled(const char *uid);

void quickplay_succeeded(const char *uid);

void quickplay_free(void);

#endif /* !WB_QUICKPLAY_H */
