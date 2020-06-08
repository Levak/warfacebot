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
#include <wb_cvar.h>


struct cb_args
{
    Warfacebot *object;
    GDBusMethodInvocation *invocation;
};

void cmd_peer_player_info_dbus_cb(const char *result,
                                  void *args)
{
    struct cb_args *a = (struct cb_args *) args;

    GVariantBuilder builder;
    GVariant *playerinfodict;

    g_variant_builder_init (&builder, G_VARIANT_TYPE("a{?*}"));

    if (result == NULL)
    {
        g_variant_builder_add (&builder, "{ss}", "error", "-1");
    }
    else
    {
        g_variant_builder_add(&builder, "{ss}", "online_id", get_info(result, "online_id='", "'", NULL) ? : "");
        g_variant_builder_add(&builder, "{ss}", "nickname", get_info(result, "nickname='", "'", NULL) ? : "");
        g_variant_builder_add(&builder, "{ss}", "primary_weapon", get_info(result, "primary_weapon='", "'", NULL) ? : "");
        g_variant_builder_add(&builder, "{ss}", "primary_weapon_skin", get_info(result, "primary_weapon_skin='", "'", NULL) ? : "");

        g_variant_builder_add(&builder, "{ss}", "banner_badge", get_info(result, "banner_badge='", "'", NULL));
        g_variant_builder_add(&builder, "{ss}", "banner_mark", get_info(result, "banner_mark='", "'", NULL));
        g_variant_builder_add(&builder, "{ss}", "banner_stripe", get_info(result, "banner_stripe='", "'", NULL));
        g_variant_builder_add(&builder, "{ss}", "experience", get_info(result, "experience='", "'", NULL));

        g_variant_builder_add(&builder, "{ss}", "pvp_rating_rank", get_info(result, "pvp_rating_rank='", "'", NULL));
        g_variant_builder_add(&builder, "{ss}", "items_unlocked", get_info(result, "items_unlocked='", "'", NULL));
        g_variant_builder_add(&builder, "{ss}", "challenges_completed", get_info(result, "challenges_completed='", "'", NULL));
        g_variant_builder_add(&builder, "{ss}", "missions_completed", get_info(result, "missions_completed='", "'", NULL));

        g_variant_builder_add(&builder, "{ss}", "pvp_wins", get_info(result, "pvp_wins='", "'", NULL));
        g_variant_builder_add(&builder, "{ss}", "pvp_loses", get_info(result, "pvp_loses='", "'", NULL));
        g_variant_builder_add(&builder, "{ss}", "pvp_total_matches", get_info(result, "pvp_total_matches='", "'", NULL));
        g_variant_builder_add(&builder, "{ss}", "pvp_kills", get_info(result, "pvp_kills='", "'", NULL));
        g_variant_builder_add(&builder, "{ss}", "pvp_deaths", get_info(result, "pvp_deaths='", "'", NULL));

        g_variant_builder_add(&builder, "{ss}", "playtime_seconds", get_info(result, "playtime_seconds='", "'", NULL));
        g_variant_builder_add(&builder, "{ss}", "leavings_percentage", get_info(result, "leavings_percentage='", "'", NULL));

        g_variant_builder_add(&builder, "{ss}", "coop_climbs_performed", get_info(result, "coop_climbs_performed='", "'", NULL));
        g_variant_builder_add(&builder, "{ss}", "coop_assists_performed", get_info(result, "coop_assists_performed='", "'", NULL));
        g_variant_builder_add(&builder, "{ss}", "favorite_pvp_class", get_info(result, "favorite_pvp_class='", "'", NULL));
        g_variant_builder_add(&builder, "{ss}", "favorite_pve_class", get_info(result, "favorite_pve_class='", "'", NULL));

        g_variant_builder_add(&builder, "{ss}", "clan_name", get_info(result, "clan_name='", "'", NULL) ? : "");
        g_variant_builder_add(&builder, "{ss}", "clan_role", get_info(result, "clan_role='", "'", NULL) ? : "");
        g_variant_builder_add(&builder, "{ss}", "clan_position", get_info(result, "clan_position='", "'", NULL) ? : "");
        g_variant_builder_add(&builder, "{ss}", "clan_points", get_info(result, "clan_points='", "'", NULL) ? : "");
        g_variant_builder_add(&builder, "{ss}", "clan_member_since", get_info(result, "clan_member_since='", "'", NULL) ? : "");
    }

    playerinfodict = g_variant_builder_end(&builder);

    warfacebot_complete_player_peer_info(
        a->object,
        a->invocation,
        playerinfodict);

    g_free(a);
}

void cmd_profile_info_get_status_dbus_cb(const char *info, void *args)
{
    struct cb_args *a = (struct cb_args *) args;

    if (info == NULL)
    {
        GVariantBuilder builder;
        GVariant *playerinfodict;

        g_variant_builder_init(&builder, G_VARIANT_TYPE("a{?*}"));
        g_variant_builder_add(&builder, "{ss}", "error", "-2");
        playerinfodict = g_variant_builder_end(&builder);

        warfacebot_complete_player_peer_info(
            a->object,
            a->invocation,
            playerinfodict);

        g_free(a);
    }
    else
    {
        char *online_id = get_info(info, "online_id='", "'", NULL);
        xmpp_iq_peer_player_info(online_id, cmd_peer_player_info_dbus_cb, a);

        free(online_id);
    }
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
        char *online_id;
        FORMAT(online_id, "%s@%s/%s", arg_NickOrOID, cvar.online_host, cvar.online_resource);

        xmpp_iq_peer_player_info(online_id, cmd_peer_player_info_dbus_cb, a);

        free(online_id);
    }
    else
    {
        xmpp_iq_profile_info_get_status(arg_NickOrOID, cmd_profile_info_get_status_dbus_cb, a);
    }

    return TRUE;
}
