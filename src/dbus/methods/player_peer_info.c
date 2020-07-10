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

void cmd_peer_player_info_dbus_cb(const struct player_peer_info *ppi,
                                  void *args)
{
    struct cb_args *a = (struct cb_args *) args;

    GVariantBuilder builder;
    GVariant *playerinfodict;

    g_variant_builder_init(&builder, G_VARIANT_TYPE("a{sv}"));

    if (ppi == NULL)
    {
        g_variant_builder_add(&builder, "{sv}", "error", g_variant_new_int32(1));
    }
    else
    {
        g_variant_builder_add(&builder, "{sv}", "online_id", g_variant_new_string(ppi->online_id ? ppi->online_id : ""));
        g_variant_builder_add(&builder, "{sv}", "nickname", g_variant_new_string(ppi->nickname ? ppi->nickname : ""));
        g_variant_builder_add(&builder, "{sv}", "primary_weapon", g_variant_new_string(ppi->primary_weapon ? ppi->primary_weapon : ""));
        g_variant_builder_add(&builder, "{sv}", "primary_weapon_skin", g_variant_new_string(ppi->primary_weapon_skin ? ppi->primary_weapon_skin : ""));

        g_variant_builder_add(&builder, "{sv}", "banner_badge", g_variant_new_int32(ppi->banner_badge));
        g_variant_builder_add(&builder, "{sv}", "banner_mark", g_variant_new_int32(ppi->banner_mark));
        g_variant_builder_add(&builder, "{sv}", "banner_stripe", g_variant_new_int32(ppi->banner_stripe));
        g_variant_builder_add(&builder, "{sv}", "experience", g_variant_new_int32(ppi->experience));

        g_variant_builder_add(&builder, "{sv}", "pvp_rating_rank", g_variant_new_int32(ppi->pvp_rating_rank));
        g_variant_builder_add(&builder, "{sv}", "items_unlocked", g_variant_new_int32(ppi->items_unlocked));
        g_variant_builder_add(&builder, "{sv}", "challenges_completed", g_variant_new_int32(ppi->challenges_completed));
        g_variant_builder_add(&builder, "{sv}", "missions_completed", g_variant_new_int32(ppi->missions_completed));

        g_variant_builder_add(&builder, "{sv}", "pvp_wins", g_variant_new_int32(ppi->pvp_wins));
        g_variant_builder_add(&builder, "{sv}", "pvp_loses", g_variant_new_int32(ppi->pvp_loses));
        g_variant_builder_add(&builder, "{sv}", "pvp_total_matches", g_variant_new_int32(ppi->pvp_total_matches));
        g_variant_builder_add(&builder, "{sv}", "pvp_kills", g_variant_new_int32(ppi->pvp_kills));
        g_variant_builder_add(&builder, "{sv}", "pvp_deaths", g_variant_new_int32(ppi->pvp_deaths));

        g_variant_builder_add(&builder, "{sv}", "playtime_seconds", g_variant_new_int32(ppi->playtime_seconds));
        g_variant_builder_add(&builder, "{sv}", "leavings_percentage", g_variant_new_double(ppi->leavings_percentage));

        g_variant_builder_add(&builder, "{sv}", "coop_climbs_performed", g_variant_new_int32(ppi->coop_climbs_performed));
        g_variant_builder_add(&builder, "{sv}", "coop_assists_performed", g_variant_new_int32(ppi->coop_assists_performed));
        g_variant_builder_add(&builder, "{sv}", "favorite_pvp_class", g_variant_new_int32(ppi->favorite_pvp_class));
        g_variant_builder_add(&builder, "{sv}", "favorite_pve_class", g_variant_new_int32(ppi->favorite_pve_class));

        g_variant_builder_add(&builder, "{sv}", "clan_name", g_variant_new_string(ppi->clan_name ? ppi->clan_name : ""));
        g_variant_builder_add(&builder, "{sv}", "clan_role", g_variant_new_int32(ppi->clan_role ? ppi->clan_role : 0));
        g_variant_builder_add(&builder, "{sv}", "clan_position", g_variant_new_int32(ppi->clan_position ? ppi->clan_position : 0));
        g_variant_builder_add(&builder, "{sv}", "clan_points", g_variant_new_int32(ppi->clan_points ? ppi->clan_points : 0));
        g_variant_builder_add(&builder, "{sv}", "clan_member_since", g_variant_new_string(ppi->clan_member_since ? ppi->clan_member_since : ""));
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

        g_variant_builder_init(&builder, G_VARIANT_TYPE("a{sv}"));
        g_variant_builder_add(&builder, "{sv}", "error", g_variant_new_int32(1));
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
