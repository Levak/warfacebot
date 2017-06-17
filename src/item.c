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

#include <wb_item.h>
#include <wb_list.h>
#include <wb_session.h>
#include <wb_xmpp_wf.h>
#include <wb_dbus.h>

#include <stdlib.h>
#include <string.h>

static int item_cmp(const struct game_item *i,
                    const char *name)
{
    return strcmp(i->name, name);
}

static int item_cmp_id(const struct game_item *i,
                       unsigned int *id)
{
    return i->id - *id;
}

static void item_free(struct game_item *i)
{
    free(i->name);
    free(i->config);

    free(i);
}

struct game_item *item_list_get(struct list *l, const char *name)
{
    if (l == NULL || name == NULL)
        return NULL;

    return list_get(l, name);
}

struct game_item *item_list_get_by_id(struct list *l, unsigned int id)
{
    if (l == NULL)
        return NULL;

    return list_get_by(l, &id, (f_list_cmp) item_cmp_id);
}

struct list *item_list_new(void)
{
    return list_new((f_list_cmp) item_cmp,
                    (f_list_free) item_free);
}

struct cb_args
{
    f_pil_update_cb fun;
    void *args;
};

static void cb(void *args)
{
    struct cb_args *a = (struct cb_args *) args;

    if (a->fun != NULL)
        a->fun(a->args);

    free(a);
}

void profile_item_list_update(f_pil_update_cb fun, void *args)
{
    struct cb_args *a = calloc(1, sizeof (struct cb_args));
    a->fun = fun;
    a->args = args;

    xmpp_iq_resync_profile(cb, a);
}

void profile_item_list_init(struct list *items)
{
    session.profile.items = items;
}

void profile_item_list_free(void)
{
    if (session.profile.items != NULL)
        list_free(session.profile.items);

    session.profile.items = NULL;
}


struct game_item *profile_item_list_get(const char *name)
{
    return item_list_get(session.profile.items, name);
}

struct game_item *profile_item_list_get_by_id(unsigned int id)
{
    return item_list_get_by_id(session.profile.items, id);
}
