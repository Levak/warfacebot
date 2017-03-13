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

#include <stdlib.h>

char *base64encode(const char *input, size_t inlength)
{
    static char b64_lkp[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    char *buffer = malloc(((inlength + 2) / 3 * 4) + 2);
    char *dest = buffer;

    while (inlength >= 3)
    {
        unsigned char b1 = *(input++);
        unsigned char b2 = *(input++);
        unsigned char b3 = *(input++);

        *(dest++) = b64_lkp[b1 >> 2];
        *(dest++) = b64_lkp[((b1 & 0x03) << 4) | (b2 >> 4)];
        *(dest++) = b64_lkp[((b2 & 0x0F) << 2) | (b3 >> 6)];
        *(dest++) = b64_lkp[b3 & 0x3F];

        inlength -= 3;
    }

    if (inlength > 0)
    {
        unsigned char b1 = *(input++);
        *(dest++) = b64_lkp[b1 >> 2];

        if (inlength == 1)
        {
            *(dest++) = b64_lkp[(b1 & 0x03) << 4];
            *(dest++) = '=';
        }
        else
        {
            unsigned char b2 = *input;
            *(dest++) = b64_lkp[((b1 & 0x03) << 4) | (b2 >> 4)];
            *(dest++) = b64_lkp[((b2 & 0x0F) << 2)];
        }

        *(dest++) = '=';
    }

    *dest = 0;

    return buffer;
}

char *base64decode(const char *input, size_t inlength, size_t *outlength)
{
    size_t posi = 0;
    size_t poso = 0;
    size_t len = ((inlength >> 2) + 3) * 3 + 1;
    char *buffer = malloc(len + 1);

    while (input[posi] && input[posi] != '=')
    {
        char res[4] = { 0 };
        int i = 0;

        for (; i < 4 && input[posi] != '='; ++i)
        {
            char c = input[posi++];

            if (c >= 'A' && c <= 'Z')
                res[i] = c - 'A';
            else if (c >= 'a' && c <= 'z')
                res[i] = c + 26 - 'a';
            else if (c >= '0' && c <= '9')
                res[i] = c + 52 - '0';
            else if (c == '+')
                res[i] = 62;
            else if (c == '/')
                res[i] = 63;
        }

        buffer[poso++] = (res[0] << 2) | (res[1] >> 4);

        if (i > 2)
            buffer[poso++] = (res[1] << 4) | (res[2] >> 2);

        if (i > 3)
            buffer[poso++] = (res[2] << 6) | res[3];
    }

    buffer[poso] = 0;

    if (outlength)
        *outlength = poso;

    return buffer;
}
