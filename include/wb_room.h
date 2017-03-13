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

#ifndef WB_ROOM_H
# define WB_ROOM_H

enum e_chat_room_type
{
    CHAT_ROOM_OTHER,
    CHAT_ROOM_GLOBAL,
    CHAT_ROOM_CLAN,
    CHAT_ROOM_PVE,
    CHAT_ROOM_PVP
};

struct room
{
    char *jid;
    enum e_chat_room_type type;
};

void room_list_add(const char *jid);

void room_list_remove(const char *jid);
void room_list_empty(void);
void room_list_init(void);
void room_list_free(void);

#endif /* !WB_ROOM_H */
