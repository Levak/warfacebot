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
#include <wb_xmpp_wf.h>
#include <wb_tools.h>
#include <wb_xmpp.h>


struct cb_args
{
    Warfacebot *object;
    GDBusMethodInvocation *invocation;
};

void cmd_peer_player_info_dbus_cb(const char *result,
                                  void *args)
{
    struct cb_args *a = (struct cb_args *) args;

    if (result == NULL)
    {
      result = "-1";
    }

    warfacebot_complete_player_peer_info(
        a->object,
        a->invocation,
        result);

    g_free(a);
}

static void cmd_profile_info_get_status_dbus_cb(const char *info, void *args)
{
    struct cb_args *a = (struct cb_args *) args;

    if (info == NULL)
    {
        warfacebot_complete_player_peer_info(
            a->object,
            a->invocation,
            "-2");

        return;
    }

    char *online_id = get_info(info, "online_id='", "'", NULL);
    xmpp_iq_peer_player_info(online_id, cmd_peer_player_info_dbus_cb, a);

    free(a);
    free(online_id);
}

/*
** DBus method call: "PlayerPeerInfo"
*/
gboolean on_handle_player_peer_info(Warfacebot *object,
                                   GDBusMethodInvocation *invocation,
                                   const gchar *arg_NickOrOID)
{
    struct cb_args *a = g_new0(struct cb_args, 1);

    a->object = object;
    a->invocation = invocation;

    gboolean is_online_id = TRUE;
    size_t len = strlen(arg_NickOrOID);
    size_t i = 0;

    for (; i < len; ++i)
        is_online_id &= g_ascii_isdigit(arg_NickOrOID[i]);


    if(is_online_id)
    {

        char *postfix = "@warface/GameClient";
        char *online_id = (char *) malloc(1 + strlen(arg_NickOrOID) + strlen(postfix));

        strcpy(online_id, arg_NickOrOID);
        strcat(online_id, postfix);

        xmpp_iq_peer_player_info(online_id, cmd_peer_player_info_dbus_cb, a);

    }else
    {
        xmpp_iq_profile_info_get_status(arg_NickOrOID, cmd_profile_info_get_status_dbus_cb, a);

    }

    return TRUE;
}
