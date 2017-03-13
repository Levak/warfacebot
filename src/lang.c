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

#include <wb_lang.h>
#include <stdarg.h>
#include <stdio.h>

struct lang lang = {
#define XLANG(Name) { .value = "@" #Name, .is_set = 0 },
    LANG_LIST
#undef XLANG
};

inline char *lang_get(const char *fmt, ...)
{
    size_t len;
    char *s;

    va_list ap;
    va_list ap2;

    va_start(ap, fmt);

    va_copy(ap2, ap);

    len = vsnprintf(NULL, 0, fmt, ap);

    s = malloc(len + 1);

    vsnprintf(s, len + 1, fmt, ap2);

    va_end(ap);

    va_end(ap2);

    return s;
}
