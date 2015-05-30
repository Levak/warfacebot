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

#ifndef WB_TOOLS_H
# define WB_TOOLS_H

# include <sys/types.h>

# define FORMAT(s, fmt, ...) do {                               \
        s = malloc(1 + snprintf(NULL, 0, fmt, __VA_ARGS__));    \
        sprintf(s, fmt, __VA_ARGS__);                           \
    } while (0)

char *get_info(const char *input,
               const char *patt_b,
               const char *patt_e,
               const char *desc);

char *get_info_first(const char *input,
                     const char *patt_b,
                     const char *patt_e_list,
                     const char *desc);

long long int get_info_int(const char *input,
                           const char *patt_b,
                           const char *patt_e,
                           const char *desc);

char *base64encode(const void *input, size_t inlength);
char *base64decode(const void *input, size_t inlength, size_t *outlength);

char *zlibb64encode(const void *input, size_t inlength);
char *zlibb64decode(const void *input, size_t inlength, size_t outlength);

#endif /* !WB_TOOLS_H */
