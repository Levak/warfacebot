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
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>
#include <wb_session.h>
#include <wb_list.h>
#include <wb_quickplay.h>
#include <wb_querycache.h>
#include <wb_cvar.h>

static int map_cmp(struct quickplay_map *m, char *mission)
{
    return strcmp(m->mission, mission);
}

static void map_free(struct quickplay_map *m)
{
    free(m->mission);
    free(m);
}

static void _parse_map(struct querycache *cache,
                       const char *elt)
{
    struct list *quickplay_maplist = (struct list *) cache->container;
    struct quickplay_map *m = calloc(1, sizeof (struct quickplay_map));

    m->mission = get_info(elt, "mission='", "'", NULL);

    const struct quickplay_map *m2 = list_get(quickplay_maplist, m->mission);

    /* new map doesn't exist in the list, add it */
    if (m2 == NULL)
        list_add(quickplay_maplist, m);
    else
        map_free(m);
}

void _reset_quickplay_maplist(void)
{
    if (session.quickplay.maps.list != NULL)
        list_free(session.quickplay.maps.list);

    session.quickplay.maps.list = list_new((f_list_cmp) map_cmp,
                                              (f_list_free) map_free);
}

void querycache_quickplay_maplist_init(void)
{
    if (cvar.query_disable_quickplay_maplist)
        return;

    querycache_init((struct querycache *) &session.quickplay.maps,
               "quickplay_maplist",
               (f_querycache_parser) _parse_map,
               (f_querycache_reset) _reset_quickplay_maplist);
}

void querycache_quickplay_maplist_free(void)
{
    if (session.quickplay.maps.list != NULL)
        list_free(session.quickplay.maps.list);
    session.quickplay.maps.list = NULL;

    querycache_free((struct querycache *) &session.quickplay.maps);
}

void xmpp_iq_quickplay_maplist(f_quickplay_maplist_cb cb, void *args)
{
    if (cvar.query_disable_quickplay_maplist)
        return;

    querycache_request((struct querycache *) &session.quickplay.maps,
                       QUERYCACHE_CURRENT_CHANNEL,
                       cb,
                       args);
}
