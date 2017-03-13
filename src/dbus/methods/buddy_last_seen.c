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
};

void cmd_last_dbus_cb(const struct cmd_last_data *last,
                      void *args)
{
    struct cb_args *a = (struct cb_args *) args;
    const char *gvariant_format = "(su)";
    GVariant *result;

    if (last != NULL)
    {
        result = g_variant_new(gvariant_format,
                               last->profile_id,
                               last->timestamp);
    }
    else
    {
        result = g_variant_new(gvariant_format,
                               "-1",
                               0);
    }

    warfacebot_complete_buddy_last_seen(
        a->object,
        a->invocation,
        result);

    g_free(a);
}

/*
** DBus method call: "BuddyLastSeen"
*/
gboolean on_handle_buddy_last_seen(Warfacebot *object,
                                   GDBusMethodInvocation *invocation,
                                   const gchar *arg_NickOrPID)
{
    struct cb_args *a = g_new0(struct cb_args, 1);

    a->object = object;
    a->invocation = invocation;

    gboolean is_profile_id = TRUE;
    size_t len = strlen(arg_NickOrPID);
    size_t i = 0;

    for (; i < len; ++i)
        is_profile_id &= g_ascii_isdigit(arg_NickOrPID[i]);

    if (is_profile_id)
        cmd_last_pid(arg_NickOrPID, cmd_last_dbus_cb, a);
    else
        cmd_last(arg_NickOrPID, cmd_last_dbus_cb, a);

    return TRUE;
}
