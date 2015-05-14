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

#include <wb_tools.h>

#include <string.h>
#include <stdlib.h>

char *wf_get_query_content(const char *msg)
{
    char *compressedData = strstr(msg, "compressedData='");

    if (!compressedData)
        return get_info(msg, "urn:cryonline:k01'>", "</query>", NULL);

    compressedData += sizeof ("compressedData='") - 1;

    char *originalSize = strstr(msg, "originalSize='");

    if (!originalSize)
        return NULL;

    originalSize += sizeof ("originalSize='") - 1;

    size_t outsize = strtol(originalSize, NULL, 10);
    size_t insize = strstr(compressedData, "'") - compressedData + 1;

    return zlibb64decode(compressedData, insize, outsize);
}
