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

#ifdef DEBUG
# include <wb_cvar.h>
#endif /* DEBUG */

#include <string.h>
#include <stdlib.h>
#include <wb_log.h>
#include <wb_querydump.h>

#define MAX_PLAIN_QUERY_SIZE 512

char *wf_get_query_content(const char *msg)
{
    if (msg == NULL)
        return NULL;

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

    char *ret = zlibb64decode(compressedData, insize, outsize);

#ifdef DEBUG
    if (ret != NULL && cvar.query_debug)
    {
        xprintf("##(%3u/%3u)-< \033[1;36m%s\033[0m",
                (unsigned) outsize, (unsigned) insize, ret);
    }

    querydump_comment(ret);
#endif

    { /* Replace any " with ' */
        for (char *s = ret; *s; ++s)
            if (*s == '"')
                *s = '\'';
    }

    return ret;
}

char *wf_compress_query(const char *iq)
{
    if (iq == NULL)
        return NULL;

    size_t total_size = strlen(iq);

    /* If query is too small to be compressed, return it as is */
    if (total_size < MAX_PLAIN_QUERY_SIZE)
        return strdup(iq);

    /* Isolate query content */
    char *query = get_info(iq, "urn:cryonline:k01'>", "</query>", NULL);

    if (query == NULL)
    {
        free(query);

        return strdup(iq);
    }

    /* Get the query name in <name/>, <name /> or <name></name> */
    char *query_name = get_info_first(query, "<", " />", NULL);

    /* If no query name is found or if the query is already compressed,
       return it as is */
    if (query_name == NULL || 0 == strcmp(query_name, "data"))
    {
        free(query);
        free(query_name);

        return strdup(iq);
    }

    char *args = get_info_first(query, query_name, ">", NULL);
    char *prologue = get_info(iq, "<", "urn:cryonline:k01'>", NULL);
    char *epilogue = get_info(iq, "</query>", "</iq>", NULL);

    /* If the query has no prologue nor epilogue, return it as is */
    if (args == NULL || prologue == NULL || epilogue == NULL)
    {
        free(args);
        free(query);
        free(query_name);
        free(prologue);
        free(epilogue);

        return strdup(iq);
    }

    /* Remove ending '/' from <foo arg1='1'/> */
    size_t end_args = strlen(args);
    if (end_args > 0 && args[end_args - 1] == '/')
        args[end_args - 1] = '\0';

    /* Compress query content */
    size_t osize = strlen(query);
    char *compressed = zlibb64encode(query, osize);

    /* Craft the compressed query */
    char *ret = NULL;
    FORMAT(ret,
           "<%surn:cryonline:k01'>"
           "<data query_name='%s'"
           " compressedData='%s'"
           " originalSize='%ld'"
           " %s />"
           "</query>%s</iq>",
           prologue,
           query_name,
           compressed,
           osize,
           args,
           epilogue);

    free(args);
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
