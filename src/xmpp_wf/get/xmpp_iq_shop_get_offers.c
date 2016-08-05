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
#include <wb_shop.h>

struct cb_args
{
    struct list *offers;
    f_shop_get_offers_cb cb;
    void *args;
};

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

static void offer_free(struct shop_offer *o)
{
    free(o->name);
    free(o->expirationTime);
    list_free(o->repairs);
    free(o);
}

static struct shop_offer *parse_offer(const char *m)
{
    struct shop_offer *o = calloc(1, sizeof (struct shop_offer));

    o->name = get_info(m, "name='", "'", NULL);

    o->id = get_info_int(m, "id='", "'", NULL);
    o->supplier_id = get_info_int(m, "supplier_id='", "'", NULL);

    o->offer_status = OFFER_NORMAL;
    {
        char *s = get_info(m, "offer_status='", "'", NULL);
        if (s != NULL)
        {
            if (0 == strcmp(s, "new"))
                o->offer_status = OFFER_NEW;
            else if (0 == strcmp(s, "sale"))
                o->offer_status = OFFER_SALE;
        }
        free(s);
    }

    o->expirationTime = get_info(m, "expirationTime='", "'", NULL);

    o->quantity = get_info_int(m, "quantity='", "'", NULL);
    o->durabilityPoints = get_info_int(m, "durabilityPoints='", "'", NULL);
    o->discount_percent = get_info_int(m, "discount='", "'", NULL);
    o->rank = get_info_int(m, "rank='", "'", NULL);

    o->price.cry.orig = get_info_int(m, "cry_price_origin='", "'", NULL);
    o->price.cry.curr = get_info_int(m, "cry_price='", "'", NULL);

    o->price.crown.orig = get_info_int(m, "crown_price_origin='", "'", NULL);
    o->price.crown.curr = get_info_int(m, "crown_price='", "'", NULL);

    o->price.game.orig = get_info_int(m, "game_price_origin='", "'", NULL);
    o->price.game.curr = get_info_int(m, "game_price='", "'", NULL);

    o->price.key.curr = get_info_int(m, "key_item_price='", "'", NULL);

    {
        char *s = get_info(m, "repair_cost='", "'", NULL);
        unsigned int repair = strtoll(s, NULL, 10);

        o->repairs = list_new((f_list_cmp) repair_cmp,
                              (f_list_free) repair_free);

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

    return o;
}

static void xmpp_iq_shop_get_offers_cb(const char *msg,
                                   enum xmpp_msg_type type,
                                   void *args)
{
    /* Answer :
       <iq to='xxx@warface/GameClient' type='result'>
        <query xmlns='urn:cryonline:k01'>
         <shop_get_offers code='2' from='0' to='250' hash='271893941'>
          <offer id='xxxxx' name='xxxx' quantity='0'
                 cry_price_origin='0' cry_price='0'
                 crown_price_origin='0' crown_price='0'
                 game_price_origin='0' game_price='0'
                 offer_status='normal' rank='0' key_item_price='0'
                 supplier_id='1' expirationTime='7 day'
                 durabilityPoints='0' discount='0'
                 repair_cost='xxx,0,0;yyyy,1,1;...' />
          ...
         </shop_get_offers>
        </query>
       </iq>
     */

    struct cb_args *a = (struct cb_args *) args;

    if (type & XMPP_TYPE_ERROR || msg == NULL)
    {
        free(a->offers);
        free(a);
        return;
    }

    char *data = wf_get_query_content(msg);

    if (data == NULL)
    {
        free(a->offers);
        free(a);
        return;
    }

    unsigned hash = get_info_int(data, "hash='", "'", NULL);
    int code = get_info_int(data, "code='", "'", NULL);
    int from = get_info_int(data, "from='", "'", NULL);
    int to = get_info_int(data, "to='", "'", NULL);

    switch (code)
    {
        case XMPP_CHUNK_MORE:
        case XMPP_CHUNK_END:
        {
            const char *m = data;

            while ((m = strstr(m, "<offer")))
            {
                char *item = get_info(m, "<offer", "/>", NULL);

                struct shop_offer *offer = parse_offer(m);

                if (offer == NULL)
                    continue;

                const struct shop_offer *o =
                    list_get(a->offers, offer->name);

                if (o == NULL)
                    list_add(a->offers, offer);
                else
                    offer_free(offer);

                free(item);
                ++m;
            }
            break;
        }

        default:
            break;
    }

    switch (code)
    {
        case XMPP_CHUNK_MORE:
            from = to;
            xmpp_send_iq_get(
                JID_MS(session.online.channel),
                xmpp_iq_shop_get_offers_cb, a,
                "<query xmlns='urn:cryonline:k01'>"
                " <shop_get_offers from='%d'/>"
                "</query>",
                from);
            break;

        case XMPP_CHUNK_END:
            if (a->cb != NULL)
                a->cb(a->offers, hash, args);
            free(a);
            break;

        default:
            free(a->offers);
            free(a);
            break;
    }

    free(data);
}

void xmpp_iq_shop_get_offers(f_shop_get_offers_cb cb,
                             void *args)
{
    struct cb_args *a = calloc(1, sizeof (struct cb_args));

    a->cb = cb;
    a->args = args;
    a->offers = list_new((f_list_cmp) offer_cmp,
                         (f_list_free) offer_free);

    xmpp_send_iq_get(
        JID_MS(session.online.channel),
        xmpp_iq_shop_get_offers_cb, a,
        "<query xmlns='urn:cryonline:k01'>"
        " <shop_get_offers/>"
        "</query>",
        NULL);
}
