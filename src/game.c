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

#include <wb_game.h>
#include <wb_cvar.h>

static enum e_server game_server;

void game_set(enum e_server server)
{
    game_server = server;

    switch (server)
    {
        case SERVER_EU:
            CVAR_STR_SET(game_version, "1.12800.3895.17500");
            CVAR_STR_SET(online_server, "com-eu.wfw.warface.com");
            break;
        case SERVER_NA:
            CVAR_STR_SET(game_version, "1.12800.3895.17500");
            CVAR_STR_SET(online_server, "com-us.wfw.warface.com");
            break;
        case SERVER_TR:
            CVAR_STR_SET(game_version, "1.12800.3895.17500");
            CVAR_STR_SET(online_server, "185.28.0.12");
            break;
        case SERVER_BR:
            CVAR_STR_SET(game_version, "1.11900.487.21500");
            CVAR_STR_SET(online_server, "game.warface.levelupgames.com.br");
            break;
        case SERVER_RU:
            CVAR_STR_SET(game_version, "1.11700.982.41200");
            CVAR_STR_SET(online_server, "s0.warface.ru"); // Alfa server
            //CVAR_STR_SET(online_server, "s1.warface.ru"); // Bravo server
            //CVAR_STR_SET(online_server, "s2.warface.ru"); // Charlie server
            break;
        case SERVER_VN:
            CVAR_STR_SET(game_version, "1.12300.428.18900");
            CVAR_STR_SET(online_server, "rrdns.warface.goplay.vn");
            break;
        default:
            break;
    }
}

void game_free(void)
{

}

inline enum e_server game_server_get(void)
{
    return game_server;
}

static const char *server_names_[] =
{
    "eu",
    "na",
    "tr",
    "br",
    "ru",
    "vn",
};

inline const char *game_server_get_str(void)
{
    return server_names_[game_server];
}
