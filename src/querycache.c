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

#include <wb_list.h>
#include <wb_querycache.h>
#include <wb_tools.h>
#include <wb_cvar.h>
#include <wb_log.h>
#include <wb_lang.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

void querycache_init(struct querycache *cache,
                     const char *queryname,
                     f_querycache_parser parser,
                     f_querycache_reset reset)
{
    if (cache == NULL || queryname == NULL)
        return;

    cache->queryname = strdup(queryname);
    cache->parser = parser;
    cache->reset = reset;
    cache->hash = NULL;

    if (cvar.query_cache && MKDIR(cvar.query_cache_location))
    {
        if (errno != EEXIST)
        {
            eprintf("%s %s",
                    LANG(error_create_directory),
                    cvar.query_cache_location);
            perror("mkdir");
        }
    }

    FORMAT(cache->filepath, "%s%s.xml",
           cvar.query_cache_location,
           cache->queryname);
    FORMAT(cache->starttag, "<%s ", cache->queryname);
    FORMAT(cache->endtag, "</%s>", cache->queryname);

    cache->file = NULL;

    querycache_load(cache);
}

void querycache_free(struct querycache *cache)
{
    if (cache == NULL)
        return;

    free(cache->hash);
    cache->hash = NULL;

    cache->parser = NULL;
    cache->reset = NULL;

    free(cache->queryname);
    cache->queryname = NULL;

    free(cache->filepath);
    cache->filepath = NULL;

    free(cache->starttag);
    cache->starttag = NULL;

    free(cache->endtag);
    cache->endtag = NULL;

    if (cache->file != NULL)
    {
        fclose(cache->file);
        cache->file = NULL;
    }
}

static void _querycache_parse(struct querycache *cache,
                              const char *str,
                              int from,
                              int to,
                              int do_write)
{
    /* Skip starttag */
    char *cache_starttag = strstr(str, cache->starttag);

    if (cache_starttag != NULL)
    {
        str += cache_starttag - str + 1;
    }

    /* While we match new tags and not the querycache endtag */
    while ((str = strstr(str, "<"))
           && str < strstr(str, cache->endtag)
           && (from < to || !do_write))
    {
        char *endnode_single = strstr(str, "/>");
        char *endnode_root = strstr(str, ">");
        char *node = NULL;

        /* Hmm? */
        if (endnode_root == NULL)
        {
            ++str;
            continue;
        }

        /* If we matched <foo...> prior to <foo.../> */
        if (endnode_single == NULL || endnode_root < endnode_single)
        {
            /* Look for </foo> */
            char *starttag = get_info_first(str, "<", " >", NULL);
            char *endtag;
            FORMAT(endtag, "</%s>", starttag);

            char *endtag_pos = strstr(endnode_root, endtag);

            node = strndup(str, endtag_pos - str + strlen(endtag));

            free(starttag);
            free(endtag);
        }
        else
        {
            /* Else Look for <foo/> */
            node = strndup(str, endnode_single - str + 2);
        }

        /* Parse a single element */
        cache->parser(cache, node);

        if (do_write && cache->file != NULL)
        {
            fprintf(cache->file, " %s\n", node);
        }

        str += strlen(node);
        free(node);

        ++from;
    }
}

void querycache_update(struct querycache *cache,
                       const char *str,
                       const char *hash,
                       int from,
                       int to,
                       int is_end)
{
    if (cache == NULL || cache->parser == NULL)
        return;

    int open_file = from == 0 && cvar.query_cache;
    int close_file = is_end && cvar.query_cache;

    /* If we matched the querycache starttag,
       open file and write starttag */
    if (open_file && cache->file == NULL)
    {
        cache->file = fopen(cache->filepath, "w");

        if (cache->file == NULL)
        {
            eprintf("%s: %s",
                    LANG(error_write_file),
                    cache->filepath);
            perror("fopen");
        }
        else
        {
            fprintf(cache->file,
                    "<%s from='%d' to='%d' hash='%s'>\n",
                    cache->queryname,
                    from,
                    to,
                    hash);
        }
    }

    if (from == 0 && cache->reset != NULL)
    {
        cache->reset();
    }

    free(cache->hash);
    cache->hash = hash ? strdup(hash) : NULL;

    _querycache_parse(cache, str, from, to, cvar.query_cache);

    /* If we matched the querycache endtag,
       write endtag and close file */
    if (close_file && cache->file != NULL)
    {
        fprintf(cache->file,
                "</%s>\n",
                cache->queryname);

        fclose(cache->file);
        cache->file = NULL;

#ifdef DEBUG
        xprintf("Cached `%s'", cache->queryname);
#endif /* DEBUG */
    }
}

void querycache_load(struct querycache *cache)
{
    if (!cvar.query_cache)
        return;

    if (cache == NULL
        || cache->parser == NULL
        || cache->filepath == NULL)
        return;

#ifdef DEBUG
    xprintf("Loading cached `%s'", cache->queryname);
#endif /* DEBUG */

    FILE *f = fopen(cache->filepath, "r");
    long fsize = 0;

    if (f == NULL)
    {
#ifdef DEBUG
        eprintf("%s: %s",
                LANG(error_read_file),
                cache->filepath);
#endif /* DEBUG */
        return;
    }

    fseek(f, 0, SEEK_END);
    fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (fsize > 0)
    {
        char *ptr = malloc(fsize + 1);

        fsize = fread(ptr, 1, fsize, f);

        if (fsize > 0)
        {
            ptr[fsize] = 0;

            char *starttag = get_info(ptr, cache->starttag, ">", NULL);
            if (starttag != NULL)
            {
                free(cache->hash);
                cache->hash = get_info(starttag, "hash='", "'", NULL);
                free(starttag);

                if (cache->reset != NULL)
                    cache->reset();

                _querycache_parse(cache, ptr, 0, 0, 0);
            }
            else
                eprintf("%s %s",
                        LANG(error_loading_file),
                        cache->filepath);
        }

        free(ptr);
    }

    fclose(f);
}
