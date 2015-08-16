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

static char *game_version;
static char *game_xmpp_server;

void game_set(enum e_server server)
{
    switch (server)
    {
        case SERVER_EU:
            game_version = "1.1.1.3624";
            game_xmpp_server = "com-eu.wfw.warface.com";
            break;
        case SERVER_NA:
            game_version = "1.1.1.3624";
            game_xmpp_server = "com-us.wfw.warface.com";
            break;
        case SERVER_TR:
            game_version = "1.1.1.3624";
            game_xmpp_server = "185.28.0.12";
            break;
        /*
        case SERVER_BR:
            game_version = TODO;
            game_xmpp_server = TODO;
            break;
        */
        case SERVER_RU:
            game_version = "1.1.1.850";
            game_xmpp_server = "s0.warface.ru"; // Alfa server
            //game_xmpp_server = "s1.warface.ru"; // Bravo server
            //game_xmpp_server = "s2.warface.ru"; // Charlie server
            break;
        case SERVER_VN:
            game_version = "1.1.1.242";
            game_xmpp_server = "rrdns.warface.goplay.vn";
            break;
        default:
            game_version = GAME_VERSION;
            game_xmpp_server = GAME_SERVER;
            break;
    }
}

inline const char *game_version_get(void)
{
    return game_version;
}

inline const char *game_xmpp_server_get(void)
{
    return game_xmpp_server;
}
