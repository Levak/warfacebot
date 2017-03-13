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
#include <zlib.h>

char *zlibb64encode(const void *input, size_t inlength)
{
    size_t len_zlib = compressBound(inlength);
    char *out_zlibc = malloc(len_zlib);
    char *out_b64c = NULL;

    compress((unsigned char *) out_zlibc, &len_zlib,
             (unsigned char *) input, inlength);
    out_b64c = base64encode(out_zlibc, len_zlib);
    free(out_zlibc);

    return out_b64c;
}

char *zlibb64decode(const void *input, size_t inlength, size_t outlength)
{
    size_t len_b64 = 0;
    size_t len_zlib = outlength;
    char *out_b64d = base64decode(input, inlength, &len_b64);
    char *out_zlibd = malloc(outlength + 1);

    if (uncompress((unsigned char *) out_zlibd, &len_zlib,
                   (unsigned char *) out_b64d, len_b64) != Z_OK)
    {
        free(out_zlibd);
        free(out_b64d);
        return NULL;
    }

    out_zlibd[outlength] = 0;
    free(out_b64d);

    return out_zlibd;
}
