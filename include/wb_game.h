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

#ifndef WB_GAME_H
# define WB_GAME_H

enum e_server
{
    SERVER_EU,
    SERVER_NA,
    SERVER_TR,
    SERVER_BR,
    SERVER_RU,
    SERVER_VN,
};

# ifndef GAME_VERSION
#  define GAME_VERSION "" /* 1.1.1.xxxx */
# endif /* !GAME_VERSION */

# ifndef GAME_XMPP_SERVER
#  define GAME_XMPP_SERVER "" /* com-xx.wfw.warface.com */
# endif /* !GAME_XMPP_SERVER */

void game_set(enum e_server server);
void game_free(void);

const char *game_version_get(void);
void game_version_set(const char *version);

const char *game_xmpp_server_get(void);
void game_xmpp_server_set(const char *server);

enum e_server game_server_get(void);

#endif /* !WB_GAME_H */
