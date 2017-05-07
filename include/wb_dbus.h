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

#ifndef WB_DBUS_H
# define WB_DBUS_H

void dbus_api_enter(const char *exe_path, const char *cmdline);

void dbus_api_setup(void);

void dbus_api_quit(int desired_exit);

/* D-Bus signal emissions bootstrap functions: */

void dbus_api_emit_room_message (
    const char *arg_Room,
    const char *arg_From,
    const char *arg_Message);

void dbus_api_emit_room_presence (
    const char *FullJid,
    int Joining);

void dbus_api_emit_room_kicked(
    int arg_Reason);

void dbus_api_emit_buddy_message (
    const char *arg_From,
    const char *arg_Message);

void dbus_api_emit_notification (
    const char *arg_Message);

void dbus_api_emit_status_update (
    const char *arg_Nickname,
    int arg_Status,
    int arg_Experience,
    int arg_ClanPoints);

void dbus_api_emit_channel_update (
    const char *Channel,
    const char *ChannelType);

/* Cached values update methods */

void dbus_api_update_buddy_list(void);
void dbus_api_update_crown_challenge(void);

#endif /* !WB_DBUS_H */
