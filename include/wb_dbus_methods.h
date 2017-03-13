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

#ifndef WB_DBUS_METHODS_H
# define WB_DBUS_METHODS_H

# include <gio/gio.h>
# include "generated-wb-api.h"

# define UNIMPLEMENTED_API_METHOD(Invocation) do {      \
        g_dbus_method_invocation_return_dbus_error (    \
            Invocation,                                 \
            "org.freedesktop.DBus.Error.UnknownMethod", \
            "*TODO* (Unimplemented Method)");           \
        return FALSE;                                   \
    } while (0)

gboolean on_handle_buddies(Warfacebot *object,
                           GDBusMethodInvocation *invocation);

gboolean on_handle_buddy_add(Warfacebot *object,
                             GDBusMethodInvocation *invocation,
                             const gchar *arg_Nickname);

gboolean on_handle_buddy_follow(Warfacebot *object,
                                GDBusMethodInvocation *invocation,
                                const gchar *arg_Nickname);

gboolean on_handle_buddy_invite(Warfacebot *object,
                                GDBusMethodInvocation *invocation,
                                const gchar *arg_Nickname);

gboolean on_handle_buddy_last_seen(Warfacebot *object,
                                   GDBusMethodInvocation *invocation,
                                   const gchar *arg_NickOrPID);

gboolean on_handle_buddy_remove(Warfacebot *object,
                                GDBusMethodInvocation *invocation,
                                const gchar *arg_Nickname);

gboolean on_handle_buddy_whisper(Warfacebot *object,
                                 GDBusMethodInvocation *invocation,
                                 const gchar *arg_Nickname,
                                 const gchar *arg_Message);

gboolean on_handle_buddy_whois(Warfacebot *object,
                               GDBusMethodInvocation *invocation,
                               const gchar *arg_Nickname);

gboolean on_handle_channel_stats(Warfacebot *object,
                                 GDBusMethodInvocation *invocation);

gboolean on_handle_channel_switch(Warfacebot *object,
                                  GDBusMethodInvocation *invocation,
                                  const gchar *arg_Channel);

gboolean on_handle_crown_challenge(Warfacebot *object,
                                   GDBusMethodInvocation *invocation);

gboolean on_handle_quit(Warfacebot *object,
                        GDBusMethodInvocation *invocation);

gboolean on_handle_room_change_map(Warfacebot *object,
                                   GDBusMethodInvocation *invocation,
                                   const gchar *arg_MapName);

gboolean on_handle_room_change_team(Warfacebot *object,
                                    GDBusMethodInvocation *invocation);

gboolean on_handle_room_give_master(Warfacebot *object,
                                    GDBusMethodInvocation *invocation,
                                    const gchar *arg_Nickname);

gboolean on_handle_room_leave(Warfacebot *object,
                              GDBusMethodInvocation *invocation);

gboolean on_handle_room_open(Warfacebot *object,
                             GDBusMethodInvocation *invocation,
                             const gchar *arg_MapName);

gboolean on_handle_room_participants(Warfacebot *object,
                                     GDBusMethodInvocation *invocation);

gboolean on_handle_room_ready(Warfacebot *object,
                              GDBusMethodInvocation *invocation);

gboolean on_handle_room_rename(Warfacebot *object,
                               GDBusMethodInvocation *invocation,
                               const gchar *arg_Name);

gboolean on_handle_room_say(Warfacebot *object,
                            GDBusMethodInvocation *invocation,
                            const gchar *arg_Message);

gboolean on_handle_room_start(Warfacebot *object,
                              GDBusMethodInvocation *invocation);

gboolean on_handle_room_take_class(Warfacebot *object,
                                   GDBusMethodInvocation *invocation,
                                   const gchar *arg_ClassName);

gboolean on_handle_chat_room_join(Warfacebot *object,
                                  GDBusMethodInvocation *invocation,
                                  const gchar *arg_Jid);

gboolean on_handle_chat_room_leave(Warfacebot *object,
                                   GDBusMethodInvocation *invocation,
                                   const gchar *arg_Jid);


#endif /* !WB_DBUS_METHODS_H */
