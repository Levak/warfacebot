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

#include <wb_game.h>

#include <stdlib.h>
#include <string.h>

static char *game_version = NULL;
static char *game_xmpp_server = NULL;
static enum e_server game_server;

void game_set(enum e_server server)
{
    game_server = server;

    switch (server)
    {
        case SERVER_EU:
            game_version_set("1.11700.3771.32100");
            game_xmpp_server_set("com-eu.wfw.warface.com");
            break;
        case SERVER_NA:
            game_version_set("1.11700.3771.32100");
            game_xmpp_server_set("com-us.wfw.warface.com");
            break;
        case SERVER_TR:
            game_version_set("1.11700.3771.32100");
            game_xmpp_server_set("185.28.0.12");
            break;
        /*
        case SERVER_BR:
            game_version_set("1.1.1.361");
            game_xmpp_server_set("game.warface.levelupgames.com.br");
            break;
        */
        case SERVER_RU:
            game_version_set("1.11700.982.41200");
            game_xmpp_server_set("s0.warface.ru"); // Alfa server
            //game_xmpp_server_set("s1.warface.ru"); // Bravo server
            //game_xmpp_server_set("s2.warface.ru"); // Charlie server
            break;
        case SERVER_VN:
            game_version_set("1.1.1.279");
            game_xmpp_server_set("rrdns.warface.goplay.vn");
            break;
        default:
            game_version_set(GAME_VERSION);
            game_xmpp_server_set(GAME_XMPP_SERVER);
            game_server = SERVER_EU;
            break;
    }
}

void game_free(void)
{
    free(game_version);
    free(game_xmpp_server);
}

inline const char *game_version_get(void)
{
    return game_version;
}

inline void game_version_set(const char *version)
{
    if (version != NULL)
    {
        free(game_version);
        game_version = strdup(version);
    }
}

inline const char *game_xmpp_server_get(void)
{
    return game_xmpp_server;
}

inline void game_xmpp_server_set(const char *server)
{
    if (server != NULL)
    {
        free(game_xmpp_server);
        game_xmpp_server = strdup(server);
    }
}

inline enum e_server game_server_get(void)
{
    return game_server;
}
