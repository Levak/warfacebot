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

#include <stdlib.h>
#include <string.h>

char *sasl_combine_logins(const char *login, const char *pwd)
{
    int lsize = strlen(login);
    int msize = strlen(pwd);
    int size = lsize + msize + 2;
    char *logins = calloc(size + 1, 1);

    strcpy(logins + 1, login);
    strcpy(logins + lsize + 2, pwd);

    char *encoded = base64encode(logins, size);

    free(logins);
    return encoded;
}
