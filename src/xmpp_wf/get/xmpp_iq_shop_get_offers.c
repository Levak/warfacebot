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
#include <wb_shop.h>
#include <wb_querycache.h>
#include <wb_cvar.h>

static void _parse_offer(struct querycache *cache,
                         const char *elt)
{
    /*
      <offer id='xxxxx' name='xxxx' quantity='0'
             cry_price_origin='0' cry_price='0'
             crown_price_origin='0' crown_price='0'
             game_price_origin='0' game_price='0'
             offer_status='normal' rank='0' key_item_price='0'
             supplier_id='1' expirationTime='7 day'
             durabilityPoints='0' discount='0'
             repair_cost='xxx,0,0;yyyy,1,1;...' />
     */
    struct list *offers = (struct list *) cache->container;

    unsigned int id = get_info_int(elt, " id='", "'", NULL);

    const struct shop_offer *o2 = offer_list_get_by_id(id); /* TODO */

    /* New offer doesn't exist in the list, add it */
    if (o2 == NULL)
    {
        struct shop_offer *o = calloc(1, sizeof (struct shop_offer));

        o->name = get_info(elt, "name='", "'", NULL);
        o->id = id;
        o->supplier_id = get_info_int(elt, "supplier_id='", "'", NULL);

        o->offer_status = OFFER_NORMAL;
        {
            char *s = get_info(elt, "offer_status='", "'", NULL);
            if (s != NULL)
            {
                if (0 == strcmp(s, "new"))
                    o->offer_status = OFFER_NEW;
                else if (0 == strcmp(s, "sale"))
                    o->offer_status = OFFER_SALE;
            }
            free(s);
        }

        o->expirationTime = get_info(elt, "expirationTime='", "'", NULL);

        o->quantity = get_info_int(elt, "quantity='", "'", NULL);
        o->durabilityPoints = get_info_int(elt, "durabilityPoints='", "'", NULL);
        o->discount_percent = get_info_int(elt, "discount='", "'", NULL);
        o->rank = get_info_int(elt, "rank='", "'", NULL);

        o->price.cry.orig = get_info_int(elt, "cry_price_origin='", "'", NULL);
        o->price.cry.curr = get_info_int(elt, "cry_price='", "'", NULL);

        o->price.crown.orig = get_info_int(elt, "crown_price_origin='", "'", NULL);
        o->price.crown.curr = get_info_int(elt, "crown_price='", "'", NULL);

        o->price.game.orig = get_info_int(elt, "game_price_origin='", "'", NULL);
        o->price.game.curr = get_info_int(elt, "game_price='", "'", NULL);

        o->price.key.curr = get_info_int(elt, "key_item_price='", "'", NULL);

        {
            char *s = get_info(elt, "repair_cost='", "'", NULL);
            unsigned int repair = strtoll(s, NULL, 10);

            o->repairs = repair_list_new();

            if (0 == strcmp(s, "0") || repair != 0)
            {
                struct repair_cost_item *r =
                    calloc(1, sizeof (struct repair_cost_item));

                if (o->name != NULL)
                    r->name = strdup(o->name);

                r->durability = o->durabilityPoints;
                r->repair_cost = repair;

                list_add(o->repairs, r);
            }
            else
            {
                char *saveptr;
                char *item = get_token(s, ";", &saveptr);

                while (item != NULL)
                {
                    struct repair_cost_item *r =
                        calloc(1, sizeof (struct repair_cost_item));

                    char *saveptr2;

                    r->name = get_token(item, ",", &saveptr2);

                    char *repair_cost = get_token(NULL, ",", &saveptr2);
                    char *durability = get_token(NULL, ",", &saveptr2);

                    if (repair_cost != NULL)
                        r->repair_cost = strtoll(repair_cost, NULL, 10);
                    if (durability != NULL)
                        r->durability = strtoll(durability, NULL, 10);

                    free(repair_cost);
                    free(durability);

                    list_add(o->repairs, r);

                    free(item);
                    item = get_token(NULL, ";", &saveptr);
                }
            }

            free(s);
        }

        list_add(offers, o);
    }
}

void _reset_shop(void)
{
    if (session.wf.shop.offers != NULL)
        list_free(session.wf.shop.offers);

    session.wf.shop.offers = offer_list_new();
}

void querycache_shop_get_offers_init(void)
{
    if (cvar.query_disable_items)
        return;

    querycache_init((struct querycache *) &session.wf.shop,
               "shop_get_offers",
               (f_querycache_parser) _parse_offer,
               (f_querycache_reset) _reset_shop);
}

void querycache_shop_get_offers_free(void)
{
    if (session.wf.shop.offers != NULL)
        list_free(session.wf.shop.offers);
    session.wf.shop.offers = NULL;

    querycache_free((struct querycache *) &session.wf.shop);
}

void xmpp_iq_shop_get_offers(f_shop_get_offers_cb cb,
                             void *args)
{
    if (cvar.query_disable_shop_get_offers)
        return;

    querycache_request((struct querycache *) &session.wf.shop,
                       QUERYCACHE_ANY_CHANNEL,
                       cb,
                       args);
}
