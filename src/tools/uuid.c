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

#include <wb_tools.h>

char *new_random_uuid(void)
{
    char *s;
    unsigned int i = 0;
    unsigned char uuid[14];

    for (; i < sizeof (uuid); ++i)
        uuid[i] = rand() % 256 + 128;

    uuid[4] = (uuid[4] & 0xF) | 0x40;
    uuid[6] = (uuid[6] & 0xF) | 0x80;

    FORMAT(s,
           "%02x%02x%02x%02x-"
           "%02x%02x-"
           "%02x%02x-"
           "%02x%02x%02x%02x%02x%02x",
           uuid[0], uuid[1], uuid[2], uuid[3],

           uuid[4], uuid[5],

           uuid[6], uuid[7],

           uuid[8], uuid[9], uuid[10], uuid[11],
           uuid[12], uuid[13]);

    return s;
}
