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
#include <wb_cmd.h>

struct cb_args
{
    Warfacebot *object;
    GDBusMethodInvocation *invocation;
    GVariantBuilder *builder;
};

static void mlist_to_array(const char *resource, int online, void *args)
{
    struct cb_args *a = (struct cb_args *) args;

    /* Is it the last one? */
    if (resource == NULL)
    {
        GVariant *marr = g_variant_new("a(si)", a->builder);

        g_variant_builder_unref(a->builder);

        warfacebot_complete_channel_stats(a->object, a->invocation, marr);

        g_free(a);
    }
    else
    {
        g_variant_builder_add(
            a->builder,
            "(si)",
            resource,
            online);
    }
}

/*
** DBus method call: "ChannelStats"
*/
gboolean on_handle_channel_stats(Warfacebot *object,
                                 GDBusMethodInvocation *invocation)
{
    struct cb_args *a = g_new0(struct cb_args, 1);
    GVariantBuilder *marr_builder;

    marr_builder = g_variant_builder_new(G_VARIANT_TYPE ("a(si)"));

    a->invocation = invocation;
    a->object = object;
    a->builder = marr_builder;

    cmd_stats(mlist_to_array, a);

    return TRUE;
}
