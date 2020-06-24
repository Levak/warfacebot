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

#include <gio/gio.h>

#include <wb_dbus_methods.h>

#include <wb_session.h>
#include <wb_shop.h>
#include <wb_xmpp_wf.h>

struct cb_args
{
    Warfacebot *object;
    GDBusMethodInvocation *invocation;
};

static void rlist_to_array(struct repair_cost_item *rci, GVariantBuilder *builder)
{
    GVariantBuilder *repair_item_builder;

    repair_item_builder = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));
    g_variant_builder_add(repair_item_builder, "{sv}", "name", g_variant_new_string(rci->name));
    g_variant_builder_add(repair_item_builder, "{sv}", "repair_cost", g_variant_new_int32(rci->repair_cost));
    g_variant_builder_add(repair_item_builder, "{sv}", "durability", g_variant_new_int32(rci->durability));

    g_variant_builder_add(builder, "a{sv}", repair_item_builder);

    g_variant_builder_unref(repair_item_builder);
}

static void olist_to_array(struct shop_offer *so, GVariantBuilder *builder)
{
    GVariantBuilder *item_builder, *repairs_builder;
    GVariant *repairs_dict_array;

    struct list *rl = so->repairs;

    item_builder = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));

    g_variant_builder_add(item_builder, "{sv}", "id", g_variant_new_int32(so->id));
    g_variant_builder_add(item_builder, "{sv}", "name", g_variant_new_string(so->name));

    g_variant_builder_add(item_builder, "{sv}", "offer_status", g_variant_new_string(
                                                                            so->offer_status == OFFER_NORMAL ? "normal":
                                                                            so->offer_status == OFFER_SALE ? "sale":
                                                                            so->offer_status == OFFER_NEW ? "new": "hot"));

    g_variant_builder_add(item_builder, "{sv}", "supplier_id", g_variant_new_int32(so->supplier_id));
    g_variant_builder_add(item_builder, "{sv}", "quantity", g_variant_new_int32(so->quantity));

    g_variant_builder_add(item_builder, "{sv}", "expirationTime", g_variant_new_string(so->expirationTime));
    g_variant_builder_add(item_builder, "{sv}", "durabilityPoints", g_variant_new_int32(so->durabilityPoints));
    g_variant_builder_add(item_builder, "{sv}", "discount_percent", g_variant_new_int32(so->discount_percent));
    g_variant_builder_add(item_builder, "{sv}", "rank", g_variant_new_int32(so->rank));
    g_variant_builder_add(item_builder, "{sv}", "key_item_name", g_variant_new_string(so->key_item_name));

    g_variant_builder_add(item_builder, "{sv}", "game_price", g_variant_new_int32(so->price.game.curr));
    g_variant_builder_add(item_builder, "{sv}", "game_price_origin", g_variant_new_int32(so->price.game.orig));

    g_variant_builder_add(item_builder, "{sv}", "cry_price", g_variant_new_int32(so->price.cry.curr));
    g_variant_builder_add(item_builder, "{sv}", "cry_price_origin", g_variant_new_int32(so->price.cry.curr));

    g_variant_builder_add(item_builder, "{sv}", "crown_price", g_variant_new_int32(so->price.crown.curr));
    g_variant_builder_add(item_builder, "{sv}", "crown_price_origin", g_variant_new_int32(so->price.crown.curr));

    g_variant_builder_add(item_builder, "{sv}", "current_price", g_variant_new_int32(so->price.key.curr));

    repairs_builder = g_variant_builder_new(G_VARIANT_TYPE("aa{sv}"));
    list_foreach(rl, (f_list_callback) rlist_to_array, repairs_builder);
    repairs_dict_array = g_variant_builder_end(repairs_builder);

    g_variant_builder_add(item_builder, "{sv}", "repairs", repairs_dict_array);

    g_variant_builder_add(builder, "a{sv}", item_builder);

    g_variant_builder_unref(item_builder);
}

void cmd_shop_offers_dbus_cb(const struct querycache *cache, void *args)
{
    struct cb_args *a = (struct cb_args *) args;
    struct list *so = cache->container;

    GVariantBuilder *builder, *shop_builder;
    GVariant *offers_dict_array, *shop_dict;

    builder = g_variant_builder_new(G_VARIANT_TYPE("aa{sv}"));

    list_foreach(so, (f_list_callback) olist_to_array, builder);

    offers_dict_array = g_variant_builder_end(builder);

    shop_builder = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));

    g_variant_builder_add(shop_builder, "{sv}", "shop_hash", g_variant_new_string(cache->hash));
    g_variant_builder_add(shop_builder, "{sv}", "offers", offers_dict_array);

    shop_dict = g_variant_builder_end(shop_builder);

    warfacebot_complete_shop_offers(
        a->object,
        a->invocation,
        shop_dict);

    g_free(a);
}

/*
** DBus method call: "ShopOffers"
*/
gboolean on_handle_shop_offers(Warfacebot *object,
                                   GDBusMethodInvocation *invocation)
{
    struct cb_args *a = g_new0(struct cb_args, 1);

    a->object = object;
    a->invocation = invocation;

    xmpp_iq_shop_get_offers(cmd_shop_offers_dbus_cb, a);

    return TRUE;
}
