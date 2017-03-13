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

#include <wb_shop.h>
#include <wb_list.h>
#include <wb_session.h>
#include <wb_xmpp_wf.h>
#include <wb_dbus.h>

#include <stdlib.h>
#include <string.h>


static int repair_cmp(struct repair_cost_item *r, char *name)
{
    return strcmp(r->name, name);
}

static void repair_free(struct repair_cost_item *r)
{
    free(r->name);
    free(r);
}

static int offer_cmp(struct shop_offer *o, char *name)
{
    return strcmp(o->name, name);
}

static int offer_cmp_id(struct shop_offer *o, unsigned int *id)
{
    return o->id - *id;
}

static void offer_free(struct shop_offer *o)
{
    free(o->name);
    free(o->expirationTime);
    list_free(o->repairs);
    free(o);
}

struct list *repair_list_new(void)
{
    return list_new((f_list_cmp) repair_cmp,
                    (f_list_free) repair_free);
}

struct list *offer_list_new(void)
{
    return list_new((f_list_cmp) offer_cmp,
                    (f_list_free) offer_free);
}

struct shop_offer *offer_list_get(const char *name)
{
    if (session.wf.shop.offers == NULL || name == NULL)
        return NULL;

    return list_get(session.wf.shop.offers, name);
}

struct shop_offer *offer_list_get_by_id(unsigned int id)
{
    if (session.wf.shop.offers == NULL)
        return NULL;

    return list_get_by(session.wf.shop.offers,
                       (void *) &id,
                       (f_list_cmp) offer_cmp_id);
}
