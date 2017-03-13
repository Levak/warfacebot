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

#ifndef WB_QUERYCACHE_H
# define WB_QUERYCACHE_H

# include <stdio.h>

enum querycache_mode
{
    QUERYCACHE_ANY_CHANNEL,
    QUERYCACHE_CURRENT_CHANNEL
};

struct querycache;

typedef void (*f_querycache_parser)(struct querycache *cache,
                                    const char *elt);

typedef void (*f_querycache_reset)(void);

# define QUERYCACHE(ContainerType, ContainerName)       \
    QUERYCACHE_(, ContainerType, ContainerName)

# define QUERYCACHE_(Name, ContainerType, ContainerName)        \
    struct Name                                                 \
    {                                                           \
        f_querycache_parser parser;                             \
        f_querycache_reset reset;                               \
        FILE *file;                                             \
        char *filepath;                                         \
        char *queryname;                                        \
        char *starttag;                                         \
        char *endtag;                                           \
                                                                \
        char *hash;                                             \
        ContainerType ContainerName;                            \
    }

/* Define root type */
QUERYCACHE_(querycache, void *, container);

void querycache_init(struct querycache *cache,
                const char *queryname,
                f_querycache_parser parser,
                f_querycache_reset reset);

void querycache_free(struct querycache *cache);

typedef void (*f_querycache_cb)(const struct querycache *cache,
                                void *args);

void querycache_request(struct querycache *cache,
                        enum querycache_mode mode,
                        f_querycache_cb cb,
                        void *args);

/* -- internal -- */
void querycache_update(struct querycache *cache,
                       const char *str,
                       const char *hash,
                       int from,
                       int to,
                       int is_end);

void querycache_load(struct querycache *cache);

#endif /* !WB_QUERYCACHE_H */
