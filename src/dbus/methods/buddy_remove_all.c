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
#include <wb_cmd.h>


static void remove_friend(struct friend *f, GVariantBuilder *builder)
{
	cmd_remove(f->nickname);

}

/*
** DBus method call: "BuddyRemoveAll"
*/
gboolean on_handle_buddy_remove_all(Warfacebot *object,
                           GDBusMethodInvocation *invocation)
{
    struct list *fl = session.profile.friends;

    list_foreach(fl, (f_list_callback) remove_friend, NULL);

    warfacebot_complete_buddy_remove_all(
        object,
        invocation);

    return TRUE;
}
