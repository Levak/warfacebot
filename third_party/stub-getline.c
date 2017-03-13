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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#ifdef __MINGW32__

# define DEFAULT_BUFF_LEN 256

/* Dummy implementation of getline() for mingw32 users. Supports only 256
   character-long lines */

ssize_t getline(char **lineptr, size_t *n, FILE *stream)
{
    if (lineptr == NULL || n == NULL)
    {
        errno = EINVAL;
        return -1;
    }

    size_t buff_len = DEFAULT_BUFF_LEN;

    *lineptr = realloc(*lineptr, buff_len * sizeof (char));

    char *ret = fgets(*lineptr, buff_len, stream);

    *n = strlen(*lineptr);

    return (ret == NULL) ? -1 : (ssize_t) *n;
}

#endif /* __MINGW32__ */
