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
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>
#include <wb_session.h>
#include <wb_list.h>
#include <wb_item.h>
#include <wb_querycache.h>
#include <wb_cvar.h>

static int item_cmp(struct game_item *o, char *name)
{
    return strcmp(o->name, name);
}

static void item_free(struct game_item *o)
{
    free(o->name);
    free(o);
}

static void _parse_item(struct querycache *cache,
                        const char *elt)
{
    struct list *items = (struct list *) cache->container;
    struct game_item *i = calloc(1, sizeof (struct game_item));

    i->name = get_info(elt, "name='", "'", NULL);
    i->id = get_info_int(elt, "id='", "'", NULL);
    i->locked = get_info_int(elt, "locked='", "'", NULL);

    const struct game_item *i2 = list_get(items, i->name);

    /* new item doesn't exist in the list, add it */
    if (i2 == NULL)
        list_add(items, i);
    else
        item_free(i);
}

void _reset_items(void)
{
    if (session.wf.items.list != NULL)
        list_free(session.wf.items.list);

    session.wf.items.list = list_new((f_list_cmp) item_cmp,
                                     (f_list_free) item_free);
}

void querycache_items_init(void)
{
    if (cvar.query_disable_items)
        return;

    querycache_init((struct querycache *) &session.wf.items,
               "items",
               (f_querycache_parser) _parse_item,
               (f_querycache_reset) _reset_items);
}

void querycache_items_free(void)
{
    if (session.wf.items.list != NULL)
        list_free(session.wf.items.list);
    session.wf.items.list = NULL;

    querycache_free((struct querycache *) &session.wf.items);
}

void xmpp_iq_items(f_items_cb cb, void *args)
{
    if (cvar.query_disable_items)
        return;

    querycache_request((struct querycache *) &session.wf.items,
                       cb,
                       args);
}
