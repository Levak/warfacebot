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

#ifndef WB_STATUS_H
# define WB_STATUS_H

enum status
{
    STATUS_OFFLINE   = 0,
    STATUS_ONLINE    = 1 << 0,
    STATUS_LEFT      = 1 << 1,
    STATUS_AFK       = 1 << 2,
    STATUS_LOBBY     = 1 << 3,
    STATUS_ROOM      = 1 << 4,
    STATUS_PLAYING   = 1 << 5,
    STATUS_SHOP      = 1 << 6,
    STATUS_INVENTORY = 1 << 7,
    STATUS_RATING    = 1 << 8,
    STATUS_TUTORIAL  = 1 << 9,
};

void status_set(enum status status);

#endif /* !WB_STATUS_H */
