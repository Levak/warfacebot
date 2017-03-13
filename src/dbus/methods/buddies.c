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
#include <wb_list.h>
#include <wb_friend.h>
#include <wb_clanmate.h>

static void flist_to_array(struct friend *f, GVariantBuilder *builder)
{
    g_variant_builder_add(
        builder,
        "(sii)",
        f->nickname,
        f->status,
        f->experience);
}

static void clist_to_array(struct clanmate *c, GVariantBuilder *builder)
{
    g_variant_builder_add(
        builder,
        "(sii)",
        c->nickname,
        c->status,
        c->experience);
}

static GVariant *farr = NULL;
static GVariant *carr = NULL;
static gboolean invalidated = TRUE;

/*
** Update the cached buddy list for DBus API.
*/
void dbus_api_update_buddy_list(void)
{
    invalidated = TRUE;
}

/*
** DBus method call: "Buddies"
*/
gboolean on_handle_buddies(Warfacebot *object,
                           GDBusMethodInvocation *invocation)
{
    if (invalidated)
    {
        struct list *fl = session.profile.friends;
        struct list *cl = session.profile.clanmates;

        GVariantBuilder *farr_builder;
        GVariantBuilder *carr_builder;

        if (farr != NULL)
            g_variant_unref(farr);

        if (carr != NULL)
            g_variant_unref(carr);

        farr_builder = g_variant_builder_new(G_VARIANT_TYPE ("a(sii)"));
        carr_builder = g_variant_builder_new(G_VARIANT_TYPE ("a(sii)"));

        list_foreach(fl, (f_list_callback) flist_to_array, farr_builder);
        list_foreach(cl, (f_list_callback) clist_to_array, carr_builder);

        farr = g_variant_new("a(sii)", farr_builder);
        carr = g_variant_new("a(sii)", carr_builder);

        g_variant_ref(farr);
        g_variant_ref(carr);

        g_variant_builder_unref(farr_builder);
        g_variant_builder_unref(carr_builder);

        invalidated = FALSE;
    }

    warfacebot_complete_buddies(
        object,
        invocation,
        farr,
        carr);

    return TRUE;
}
