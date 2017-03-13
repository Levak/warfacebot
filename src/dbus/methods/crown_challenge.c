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
#include <wb_mission.h>
#include <wb_session.h>
#include <wb_list.h>

static void mlist_to_array(struct mission *m, GVariantBuilder *builder)
{
    if (m->crown_time_gold == 0)
        return;

    g_variant_builder_add(
        builder,
        "(sssii)",
        m->type,
        m->setting,
        m->image,
        m->crown_time_gold,
        m->crown_perf_gold);
}

static GVariant *marr = NULL;
static gboolean invalidated = TRUE;

/*
** Update the cached crown challenge for DBus API.
*/
void dbus_api_update_crown_challenge(void)
{
    invalidated = TRUE;
}

/*
** DBus method call: "CrownChallenge"
*/
gboolean on_handle_crown_challenge(Warfacebot *object,
                                   GDBusMethodInvocation *invocation)
{
    if (invalidated)
    {
        struct list *ml = session.wf.missions.list;

        GVariantBuilder *marr_builder;

        if (marr != NULL)
            g_variant_unref(marr);

        marr_builder = g_variant_builder_new(G_VARIANT_TYPE ("a(sssii)"));

        list_foreach(ml, (f_list_callback) mlist_to_array, marr_builder);

        marr = g_variant_new("a(sssii)", marr_builder);

        g_variant_ref(marr);

        g_variant_builder_unref(marr_builder);

        invalidated = FALSE;
    }

    warfacebot_complete_crown_challenge(object, invocation, marr);

    return TRUE;
}
