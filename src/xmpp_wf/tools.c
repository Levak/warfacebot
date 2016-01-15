/**
 * WarfaceBot, a blind XMPP client for Warface (FPS)
 * Copyright (C) 2015, 2016 Levak Borok <levak92@gmail.com>
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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_PLAIN_QUERY_SIZE 256

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

char *wf_compress_query(const char *iq)
{
    if (iq == NULL)
        return NULL;

    size_t total_size = strlen(iq);

    if (total_size < MAX_PLAIN_QUERY_SIZE)
        return strdup(iq);

    char *query = get_info(iq, "urn:cryonline:k01'>", "</query>", NULL);
    char *prologue = get_info(iq, "<", "urn:cryonline:k01'>", NULL);
    char *epilogue = get_info(iq, "</query>", "</iq>", NULL);

    if (query == NULL || prologue == NULL || epilogue == NULL)
        return strdup(iq);

    size_t osize = strlen(query);
    char *compressed = zlibb64encode(query, osize);
    char *query_name = get_info_first(query, "<", " />", NULL);

    char *ret = NULL;
    FORMAT(ret,
           "<%surn:cryonline:k01'>"
           "<data query_name='%s'"
           " compressedData='%s'"
           " originalSize='%ld'"
           "/>"
           "</query>%s</iq>",
           prologue, query_name, compressed, osize, epilogue);

    free(query);
    free(prologue);
    free(epilogue);
    free(compressed);
    free(query_name);

    return ret;
}

char *wf_decompress_query(const char *iq)
{
    if (iq == NULL)
        return NULL;

    return strdup(iq);
}
