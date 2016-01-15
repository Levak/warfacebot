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

#include <gio/gio.h>

#include <wb_session.h>
#include <wb_game.h>

#include <wb_dbus.h>
#include <wb_dbus_methods.h>

#include "api-defs.h"

/*
** Instance dbus global variables
*/
static GMainLoop *loop = NULL;
static gchar *bus_name = NULL;
static Warfacebot *wb = NULL;
static guint watch_mngr;
static guint owned_bus;
static WarfacebotMngr *wbm = NULL;

inline void dbus_api_emit_room_message (
    const char *Room,
    const char *From,
    const char *Message)
{
    if (wb != NULL && Room != NULL && From != NULL && Message != NULL)
        warfacebot_emit_room_message(wb, Room, From, Message);
}

inline void dbus_api_emit_room_kicked(void)
{
    if (wb != NULL)
        warfacebot_emit_room_kicked(wb);
}

inline void dbus_api_emit_buddy_message (
    const char *From,
    const char *Message)
{
    if (wb != NULL && From != NULL && Message != NULL)
        warfacebot_emit_buddy_message(wb, From, Message);
}

inline void dbus_api_emit_notification (
    const char *Message)
{
    if (wb != NULL && Message != NULL)
        warfacebot_emit_notification(wb, Message);
}

inline void dbus_api_emit_status_update (
    const char *Nickname,
    int Status,
    int Experience,
    int ClanPoints)
{
    if (wb != NULL && Nickname != NULL)
        warfacebot_emit_status_update(wb, Nickname, Status, Experience, ClanPoints);
}

/*
** DBus event: Manager bus appeared
**  - Notify manager
*/
static void on_mngr_name_appeared(GDBusConnection *connection,
                                  const gchar *name,
                                  const gchar *name_owner,
                                  gpointer user_data)
{
    GError *error = NULL;
    gboolean ret = FALSE;

    g_print("Manager appeared: %s\n", name);

    wbm = warfacebot_mngr_proxy_new_sync(
        connection,
        0,
        API_MNGR_NAME,
        API_MNGR_PATH,
        NULL,
        &error);

    if (wbm == NULL)
    {
        g_warning(error->message);
        return;
    }

    ret = warfacebot_mngr_call_instance_ready_sync(
        wbm,
        session.nickname,
        game_server_get_str(),
        bus_name,
        NULL,
        &error);

    if (!ret)
    {
        g_warning(error->message);
        return;
    }

    g_print("Notified Manager: %s\n", bus_name);
}

/*
** DBus event: Manager bus vanished
*/
static void on_mngr_name_vanished(GDBusConnection *connection,
                                  const gchar *name,
                                  gpointer user_data)
{
    g_print("Manager vanished: %s\n", name);

    g_bus_unwatch_name(
        g_bus_watch_name(
            G_BUS_TYPE_SESSION,
            API_MNGR_NAME,
            G_BUS_NAME_WATCHER_FLAGS_AUTO_START,
            NULL,
            NULL,
            NULL,
            NULL));

    if (wbm != NULL)
        g_object_unref(wbm);

    wbm = NULL;
}

/*
** DBus event: Instance bus acquired
**  - Bind instance interface method calls
**  - Export instance interface
**  - Watch manager bus
*/
static void on_bus_acquired(GDBusConnection *connection,
                            const gchar *name,
                            gpointer user_data)
{
    GError *error = NULL;
    gboolean ret = FALSE;
    WarfacebotIface *iface = NULL;

    g_print("Bus acquired: %s\n", name);

    wb = warfacebot_skeleton_new();
    iface = WARFACEBOT_GET_IFACE (wb);

    iface->handle_buddies = on_handle_buddies;
    iface->handle_buddy_add = on_handle_buddy_add;
    iface->handle_buddy_follow = on_handle_buddy_follow;
    iface->handle_buddy_invite = on_handle_buddy_invite;
    iface->handle_buddy_remove = on_handle_buddy_remove;
    iface->handle_buddy_whisper = on_handle_buddy_whisper;
    iface->handle_buddy_whois = on_handle_buddy_whois;
    iface->handle_channel_switch = on_handle_channel_switch;
    iface->handle_crown_challenge = on_handle_crown_challenge;
    iface->handle_quit = on_handle_quit;
    iface->handle_room_change_map = on_handle_room_change_map;
    iface->handle_room_change_team = on_handle_room_change_team;
    iface->handle_room_give_master = on_handle_room_give_master;
    iface->handle_room_leave = on_handle_room_leave;
    iface->handle_room_open = on_handle_room_open;
    iface->handle_room_participants = on_handle_room_participants;
    iface->handle_room_ready = on_handle_room_ready;
    iface->handle_room_rename = on_handle_room_rename;
    iface->handle_room_say = on_handle_room_say;
    iface->handle_room_start = on_handle_room_start;
    iface->handle_room_take_class = on_handle_room_take_class;

    ret = g_dbus_interface_skeleton_export(
        G_DBUS_INTERFACE_SKELETON (wb),
        connection,
        API_INST_PATH,
        NULL);

    if (!ret)
    {
        g_warning(error->message);
        return;
    }

    watch_mngr = g_bus_watch_name(
        G_BUS_TYPE_SESSION,
        API_MNGR_NAME,
        G_BUS_NAME_WATCHER_FLAGS_AUTO_START,
        on_mngr_name_appeared,
        on_mngr_name_vanished,
        NULL,
        NULL);

    g_print("Bus created: %s\n", name);
}

/*
** DBus event: Instance bus lost
**  - Unwatch manager bus
**  - Exit
*/
void on_bus_lost(GDBusConnection *connection,
                 const gchar *name,
                 gpointer user_data)
{
    g_print("Instance bus lost: %s\n", name);

    if (wb != NULL)
        g_object_unref(wb);

    if (watch_mngr > 0)
        g_bus_unwatch_name(watch_mngr);

    if (bus_name != NULL)
        g_free(bus_name);

    dbus_api_quit(0);
}

/*
** Setup DBus API
** Once everything is ready, export the API
*/
void dbus_api_setup(void)
{
    bus_name = g_strdup_printf(
        API_INST_NAME ".%s.%s",
        game_server_get_str(),
        session.nickname);

    owned_bus = g_bus_own_name(
        G_BUS_TYPE_SESSION,
        bus_name,
        G_BUS_NAME_OWNER_FLAGS_ALLOW_REPLACEMENT |
        G_BUS_NAME_OWNER_FLAGS_REPLACE,
        on_bus_acquired,
        NULL,
        on_bus_lost,
        NULL,
        NULL);
}

/*
** Glib loop thread.
** Creates the instance bus.
*/
void dbus_api_enter(void)
{
    loop = g_main_loop_new(NULL, FALSE);

    g_main_loop_run(loop);

    g_print("Exit gloop\n");

    if (owned_bus > 0)
        g_bus_unown_name(owned_bus);

    if (loop != NULL)
        g_main_loop_unref(loop);

    if (bus_name != NULL)
        g_free(bus_name);
}

/*
** Glib loop killer
*/
void dbus_api_quit(int desired_exit)
{
    if (desired_exit && wbm != NULL)
    {
        GError *error = NULL;
        gboolean ret = FALSE;

        ret = warfacebot_mngr_call_instance_exit_sync(
            wbm,
            bus_name,
            NULL,
            &error);

        if (!ret)
            g_warning(error->message);
    }

    g_main_loop_quit(loop);
}
