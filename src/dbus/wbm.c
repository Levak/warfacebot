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
#include <stdio.h>
#include "generated-wb-api.h"
#include "api-defs.h"

/*
** Manager dbus global variables
*/
static gchar *prog_path = NULL;
static unsigned int uniq_id = 0;
static GDBusObjectManagerServer *manager = NULL;
static GDBusConnection *connection = NULL;
static GMainLoop *loop;
static GAsyncQueue *create_queue = NULL;
static GThread *create_thread = NULL;

/*
** Craft the service file path for a specific Bus name
*/
static gchar *get_service_file_path(const char *BusName)
{
    gchar *service_file_name = NULL;
    gchar *service_file_path = NULL;
    gchar *service_folder_path = NULL;

    service_folder_path = g_build_filename(
        g_get_home_dir(),
        ".local",
        "share",
        "dbus-1",
        "services",
        NULL);

    g_mkdir_with_parents(service_folder_path, 0755);

    service_file_name = g_strdup_printf("%s.service", BusName);

    service_file_path = g_build_filename(
        service_folder_path,
        service_file_name,
        NULL);

    g_free(service_file_name);
    g_free(service_folder_path);

    return service_file_path;
}

/*
** Remove the service file for a specific Bus name
*/
static void delete_service_file(const gchar *BusName)
{
    gchar *service_file_path = NULL;

    service_file_path = get_service_file_path(BusName);
    remove(service_file_path);
    g_free(service_file_path);
}

/*
** Craft a service file for a specific Bus name
*/
static void create_service_file(const gchar *RootPath,
                                const gchar *BusName,
                                const gchar *DBusID,
                                const gchar *Server,
                                const gchar *Options)
{
    FILE *service_file = NULL;
    gchar *service_file_path = NULL;

    service_file_path = get_service_file_path(BusName);
    service_file = fopen(service_file_path, "w");
    g_free(service_file_path);

    if (service_file != NULL)
    {
        gchar *exe_path = NULL;

        if (DBusID != NULL && Server != NULL)
        {
            exe_path = g_build_filename(
                RootPath ? RootPath : prog_path,
                "wbd_launcher",
                NULL);

            fprintf(
                service_file,
                "[D-BUS Service]\n"
                "Name=%s\n"
                "Exec=%s %s %s %s\n",
                BusName,
                exe_path,
                DBusID,
                Server,
                Options ? Options : "");
        }
        else
        {
            exe_path = g_build_filename(
               RootPath ? RootPath : prog_path,
               "wbm",
               NULL);

            fprintf(
                service_file,
                "[D-BUS Service]\n"
                "Name=%s\n"
                "Exec=%s %s\n",
                BusName,
                exe_path,
                Options ? Options : "");
        }

        fclose(service_file);

        g_free(exe_path);
    }
}

static gpointer create_queue_consumer_thread(gpointer data)
{
    while (create_thread != NULL)
    {
        gchar *busname = g_async_queue_try_pop(create_queue);

        if (busname != NULL)
        {
            g_bus_unwatch_name(
                g_bus_watch_name(
                    G_BUS_TYPE_SESSION,
                    busname,
                    G_BUS_NAME_WATCHER_FLAGS_AUTO_START,
                    NULL,
                    NULL,
                    NULL,
                    NULL));

            g_free(busname);
        }

        /*
        ** Wait two seconds to prevent flooding the server if
        ** all instances just vanished and they are all queued
        ** to respawn.
        */
        g_usleep(2000000L);
    }

    return NULL;
}

/*
** Tell DBus to auto-launch a daemon based on a busname
** if it doesn't exists. DBus will look for a service file
** corresponding to the given well-known name.
*/
static inline void autolaunch_process_by_busname(const gchar *busname)
{
    g_async_queue_push(create_queue, g_strdup(busname));
}

/*
** Callback structure to identify an instance watch
*/
struct watch_instance
{
    guint id;
    guint timeout_id;
    gchar *path;
    gchar *name;
};

/*
** Destroy an instance watch
*/
static void watch_instance_free(struct watch_instance *watch)
{
    if (watch->timeout_id > 0)
        g_source_remove(watch->timeout_id);
    watch->timeout_id = 0;

    if (watch->id > 0)
        g_bus_unwatch_name(watch->id);
    watch->id = 0;

    g_free(watch->path);
    watch->path = NULL;

    g_free(watch->name);
    watch->name = NULL;

    g_free(watch);
}

/*
** DBus event: An instance bus appeared
**  - If we were tracking it, destroy the watch
**    (we will recreate it later with InstanceReady)
*/
static void on_instance_name_appeared(GDBusConnection *conn,
                                      const gchar *name,
                                      const gchar *name_owner,
                                      gpointer user_data)
{
    struct watch_instance *watch = user_data;

    if (watch->timeout_id > 0)
        watch_instance_free(watch);
}

/*
** Timeout event: An instance didn't spawn in time.
**  - If there is no more service file, destroy the watch
**  - Else, make it spawn again
*/
static gboolean on_instance_timeout(gpointer user_data)
{
    struct watch_instance *watch = user_data;
    gchar *service_path = NULL;
    gboolean service_exists = FALSE;

    service_path = get_service_file_path(watch->name);
    service_exists = g_file_test(service_path, G_FILE_TEST_EXISTS);
    g_free(service_path);

    if (service_exists == FALSE)
    {
        g_print("No more service file: %s\n", watch->name);

        watch_instance_free(watch);

        return FALSE;
    }

    g_print("Still no answer from: %s\n", watch->name);

    autolaunch_process_by_busname(watch->name);

    return TRUE;
}

/*
** DBus event: An instance bus vanished
**  - Create a timeout watch
**  - Unwatch the instance bus
*/
static void on_instance_name_vanished(GDBusConnection *conn,
                                      const gchar *name,
                                      gpointer user_data)
{
    struct watch_instance *watch = user_data;
    gchar *service_path = NULL;
    gboolean service_exists = FALSE;

    g_print("Instance vanished: %s\n", watch->name);
    g_dbus_object_manager_server_unexport(manager, watch->path);

    service_path = get_service_file_path(watch->name);
    service_exists = g_file_test(service_path, G_FILE_TEST_EXISTS);
    g_free(service_path);

    if (service_exists == FALSE)
    {
        g_print("No more service file: %s\n", watch->name);

        watch_instance_free(watch);
    }
    else
    {
        g_print("Trying to restart: %s\n", name);

        watch->timeout_id = g_timeout_add_seconds(
            300,
            on_instance_timeout,
            watch);

        autolaunch_process_by_busname(watch->name);
    }
}

/*
** DBus method call: "InstanceExit" -- Called from an instance
*/
static gboolean on_handle_instance_exit(WarfacebotMngr *wbm,
                                        GDBusMethodInvocation *invocation,
                                        const gchar *BusName)
{
    g_print("Instance wants to exit: %s\n", BusName);

    if (!g_dbus_is_name(BusName))
    {
        g_dbus_method_invocation_return_dbus_error(
            invocation,
            "org.freedesktop.DBus.Error.ServiceUnknown",
            "Invalid BusName");
        return FALSE;
    }

    delete_service_file(BusName);

    warfacebot_mngr_complete_instance_exit(
        wbm,
        invocation);

    return TRUE;
}

/*
** DBus method call: "InstanceReady" -- Called from an instance
**  - Create an instance interface in the manager bus
**  - Watch the instance bus
*/
static gboolean on_handle_instance_ready(WarfacebotMngr *wbm,
                                         GDBusMethodInvocation *invocation,
                                         const gchar *DBusID,
                                         const gchar *Nickname,
                                         const gchar *Server,
                                         const gchar *BusName,
                                         const gchar *Options,
                                         const gchar *RootPath)
{
    ObjectSkeleton *skeleton = NULL;
    WarfacebotMngrInstance *wbi = NULL;
    struct watch_instance *watch = NULL;

    if (!g_dbus_is_name(BusName))
    {
        g_dbus_method_invocation_return_dbus_error(
            invocation,
            "org.freedesktop.DBus.Error.ServiceUnknown",
            "Invalid BusName");
        return FALSE;
    }

    g_print("Notification from instance: %s\n", BusName);

    watch = g_new0(struct watch_instance, 1);

    watch->name = g_strdup(BusName);
    watch->path = g_strdup_printf(API_MNGR_PATH "/%03d", ++uniq_id);
    skeleton = object_skeleton_new(watch->path);

    wbi = warfacebot_mngr_instance_skeleton_new();

    warfacebot_mngr_instance_set_dbus_id(wbi, DBusID);
    warfacebot_mngr_instance_set_nickname(wbi, Nickname);
    warfacebot_mngr_instance_set_server(wbi, Server);
    warfacebot_mngr_instance_set_bus_name(wbi, BusName);
    warfacebot_mngr_instance_set_starttime(wbi, time(NULL));

    object_skeleton_set_warfacebot_mngr_instance(skeleton, wbi);

    g_dbus_object_manager_server_export(
        manager,
        G_DBUS_OBJECT_SKELETON (skeleton));

    g_object_unref(skeleton);

    create_service_file(RootPath, BusName, DBusID, Server, Options);

    watch->id = g_bus_watch_name(
        G_BUS_TYPE_SESSION,
        BusName,
        G_BUS_NAME_WATCHER_FLAGS_AUTO_START,
        on_instance_name_appeared,
        on_instance_name_vanished,
        watch,
        NULL);

    g_print("Watching instance: %s\n", BusName);

    warfacebot_mngr_complete_instance_ready(
        wbm,
        invocation);

    return TRUE;
}

/*
** DBus method call: "Create" -- Called from a third party
**  - Spawn an instance process
*/
static gboolean on_handle_create(WarfacebotMngr *wbm,
                                 GDBusMethodInvocation *invocation,
                                 const gchar *DBusID,
                                 const gchar *Server,
                                 const gchar *Options)
{

    gchar *BusName = NULL;
    BusName = g_strdup_printf(API_INST_NAME ".%s.%s", Server, DBusID);

    g_print("Spawn service: %s\n", BusName);

    create_service_file(prog_path, BusName, DBusID, Server, Options);

    autolaunch_process_by_busname(BusName);

    g_free(BusName);

    warfacebot_mngr_complete_create(wbm, invocation);

    return TRUE;
}

/*
** DBus method call: "Quit" -- Called from a third party
**  - Remove manager service file
**  - Quit manager
*/
static gboolean on_handle_quit(WarfacebotMngr *wbm,
                               GDBusMethodInvocation *invocation)
{
    delete_service_file(API_MNGR_NAME);

    g_main_loop_quit(loop);

    warfacebot_mngr_complete_quit(wbm, invocation);

    return TRUE;
}

/*
** DBus event: Manager bus acquired
**  - Export manager interface
**  - Bind manager interface methods
**  - Create manager service file
*/
static void on_bus_acquired(GDBusConnection *conn,
                            const gchar *name,
                            gpointer user_data)
{
    WarfacebotMngr *wbm = NULL;
    WarfacebotMngrIface *iface = NULL;
    GError *error = NULL;
    gboolean ret = FALSE;

    g_print ("Manager acquired message bus %s\n", name);

    create_service_file(prog_path, API_MNGR_NAME, NULL, NULL, NULL);

    /* Create Manager */

    connection = conn;
    manager = g_dbus_object_manager_server_new(API_MNGR_PATH);

    g_dbus_object_manager_server_set_connection(manager, connection);

    /* Create Custom Interface */

    wbm = warfacebot_mngr_skeleton_new();

    iface = WARFACEBOT_MNGR_GET_IFACE (wbm);

    iface->handle_create = on_handle_create;
    iface->handle_quit = on_handle_quit;
    iface->handle_instance_exit = on_handle_instance_exit;
    iface->handle_instance_ready = on_handle_instance_ready;

    ret = g_dbus_interface_skeleton_export(
        G_DBUS_INTERFACE_SKELETON (wbm),
        connection,
        API_MNGR_PATH,
        &error);

    if (ret == FALSE)
    {
        g_warning("%s\n", error->message);
        return;
    }

    g_print("Manager interface exported\n");
}

/*
** DBus event: Manager bus lost
**  - Exit
*/
void on_bus_lost(GDBusConnection *conn,
                 const gchar *name,
                 gpointer user_data)
{
    g_print("Manager bus lost: %s\n", name);

    if (manager != NULL)
        g_object_unref(manager);

    g_main_loop_quit(loop);
}


int main(int argc, char *argv[])
{
    guint id;

    prog_path = g_path_get_dirname(argv[0]);

    if (!g_path_is_absolute(prog_path))
    {
        gchar *rel_path = prog_path;

        prog_path = g_build_filename(
            g_get_current_dir(),
            rel_path,
            NULL);

        g_free(rel_path);
    }

    loop = g_main_loop_new(NULL, FALSE);

    create_queue = g_async_queue_new();
    create_thread = g_thread_new(
        "create_queue",
        create_queue_consumer_thread,
        NULL);

    id = g_bus_own_name(
        G_BUS_TYPE_SESSION,
        API_MNGR_NAME,
        G_BUS_NAME_OWNER_FLAGS_ALLOW_REPLACEMENT |
        G_BUS_NAME_OWNER_FLAGS_REPLACE,
        on_bus_acquired,
        NULL,
        on_bus_lost,
        NULL,
        NULL);

    g_main_loop_run(loop);

    g_async_queue_unref(create_queue);
    create_queue = NULL;
    g_thread_unref(create_thread);
    create_thread = NULL;

    g_free(prog_path);
    g_bus_unown_name(id);
    g_main_loop_unref(loop);

    return 0;
}
