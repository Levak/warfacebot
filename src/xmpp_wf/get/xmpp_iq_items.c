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
#include <wb_item.h>
#include <wb_querycache.h>
#include <wb_cvar.h>

static void _parse_item(struct querycache *cache,
                        const char *elt)
{
    struct list *items = (struct list *) cache->container;

    char *name = get_info(elt, "name='", "'", NULL);

    if (name == NULL)
        return;

    const struct game_item *i2 = list_get(items, name);

    /* New item doesn't exist in the list, add it */
    if (i2 == NULL)
    {
        struct game_item *i = calloc(1, sizeof (struct game_item));

        i->name = name;
        i->id = get_info_int(elt, "id='", "'", NULL);;
        i->locked = get_info_int(elt, "locked='", "'", NULL);

        /* Ignore contracts to be "locked" */
        if (strstr(i->name, "contract") != NULL)
            i->locked = 0;

        if (i->locked != 0)
            ++session.wf.total_locked_items;

        list_add(items, i);
    }
    else
        free(name);
}

void _reset_items(void)
{
    if (session.wf.items.list != NULL)
        list_free(session.wf.items.list);

    session.wf.items.list = item_list_new();
    session.wf.total_locked_items = 0;
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
                       QUERYCACHE_ANY_CHANNEL,
                       cb,
                       args);
}
