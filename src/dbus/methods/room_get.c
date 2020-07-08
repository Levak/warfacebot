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

#include <wb_gameroom.h>

struct cb_args
{
    Warfacebot *object;
    GDBusMethodInvocation *invocation;
};

static void create_gr_sync(s_gr_sync *base, GVariantBuilder *builder)
{
    g_variant_builder_add(builder, "{sv}", "revision", g_variant_new_int32(base->revision));
}

static void create_gameroom_clan_war(s_gr_clan_war *clan_war, GVariantBuilder *builder)
{
    GVariantBuilder *clan_war_builder;
    GVariant *clan_war_dict_array;

    clan_war_builder = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));

    g_variant_builder_add(clan_war_builder, "{sv}", "clan_1", g_variant_new_string(clan_war->clan_1 ? : ""));
    g_variant_builder_add(clan_war_builder, "{sv}", "clan_2", g_variant_new_string(clan_war->clan_2 ? : ""));

    clan_war_dict_array = g_variant_builder_end(clan_war_builder);

    g_variant_builder_add(builder, "{sv}", "clan_war", clan_war_dict_array);
    g_variant_builder_unref(clan_war_builder);
}

static void create_gameroom_auto_start(s_gr_auto_start *auto_start, GVariantBuilder *builder)
{
    GVariantBuilder *auto_start_builder;
    GVariant *auto_start_dict_array;

    auto_start_builder = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));

    create_gr_sync(&auto_start->base, auto_start_builder);
    g_variant_builder_add(auto_start_builder, "{sv}", "has_timeout", g_variant_new_int32(auto_start->has_timeout));
    g_variant_builder_add(auto_start_builder, "{sv}", "timeout_left", g_variant_new_int32(auto_start->timeout_left));
    g_variant_builder_add(auto_start_builder, "{sv}", "can_manual_start", g_variant_new_int32(auto_start->can_manual_start));
    g_variant_builder_add(auto_start_builder, "{sv}", "joined_intermission_timeout", g_variant_new_int32(auto_start->joined_intermission_timeout));

    auto_start_dict_array = g_variant_builder_end(auto_start_builder);

    g_variant_builder_add(builder, "{sv}", "auto_start", auto_start_dict_array);
    g_variant_builder_unref(auto_start_builder);
}

static void create_gameroom_regions(s_gr_regions *regions, GVariantBuilder *builder)
{
    GVariantBuilder *regions_builder;
    GVariant *regions_dict_array;

    regions_builder = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));

    create_gr_sync(&regions->base, regions_builder);
    g_variant_builder_add(regions_builder, "{sv}", "region_id", g_variant_new_string(regions->region_id));

    regions_dict_array = g_variant_builder_end(regions_builder);

    g_variant_builder_add(builder, "{sv}", "regions", regions_dict_array);
    g_variant_builder_unref(regions_builder);
}

static void create_gameroom_room_master(s_gr_room_master *room_master, GVariantBuilder *builder)
{
    GVariantBuilder *room_master_builder;
    GVariant *room_master_dict_array;

    room_master_builder = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));

    create_gr_sync(&room_master->base, room_master_builder);
    g_variant_builder_add(room_master_builder, "{sv}", "master", g_variant_new_string(room_master->master));

    room_master_dict_array = g_variant_builder_end(room_master_builder);

    g_variant_builder_add(builder, "{sv}", "room_master", room_master_dict_array);
    g_variant_builder_unref(room_master_builder);
}

static void create_gameroom_session(s_gr_session *session, GVariantBuilder *builder)
{
    GVariantBuilder *session_builder;
    GVariant *session_dict_array;

    session_builder = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));

    create_gr_sync(&session->base, session_builder);
    g_variant_builder_add(session_builder, "{sv}", "id", g_variant_new_string(session->id ? : ""));
    g_variant_builder_add(session_builder, "{sv}", "status", g_variant_new_int32(session->status));
    g_variant_builder_add(session_builder, "{sv}", "game_progress", g_variant_new_int32(session->game_progress));
    g_variant_builder_add(session_builder, "{sv}", "start_time", g_variant_new_int32(session->start_time));
    g_variant_builder_add(session_builder, "{sv}", "team1_start_score", g_variant_new_int32(session->team1_start_score));
    g_variant_builder_add(session_builder, "{sv}", "team2_start_score", g_variant_new_int32(session->team2_start_score));

    session_dict_array = g_variant_builder_end(session_builder);

    g_variant_builder_add(builder, "{sv}", "session", session_dict_array);
    g_variant_builder_unref(session_builder);
}

static void create_gameroom_mission(s_gr_mission *mission, GVariantBuilder *builder)
{
    GVariantBuilder *mission_builder;
    GVariant *mission_dict_array;

    mission_builder = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));

    create_gr_sync(&mission->base, mission_builder);
    g_variant_builder_add(mission_builder, "{sv}", "mission_key", g_variant_new_string(mission->mission_key));
    g_variant_builder_add(mission_builder, "{sv}", "name", g_variant_new_string(mission->name));
    g_variant_builder_add(mission_builder, "{sv}", "setting", g_variant_new_string(mission->setting));
    g_variant_builder_add(mission_builder, "{sv}", "mode", g_variant_new_string(mission->mode));
    g_variant_builder_add(mission_builder, "{sv}", "mode_name", g_variant_new_string(mission->mode_name));
    g_variant_builder_add(mission_builder, "{sv}", "mode_icon", g_variant_new_string(mission->mode_icon));
    g_variant_builder_add(mission_builder, "{sv}", "description", g_variant_new_string(mission->description));
    g_variant_builder_add(mission_builder, "{sv}", "image", g_variant_new_string(mission->image));
    g_variant_builder_add(mission_builder, "{sv}", "difficulty", g_variant_new_string(mission->difficulty));
    g_variant_builder_add(mission_builder, "{sv}", "type", g_variant_new_string(mission->type));
    g_variant_builder_add(mission_builder, "{sv}", "time_of_day", g_variant_new_string(mission->time_of_day));
    g_variant_builder_add(mission_builder, "{sv}", "no_teams", g_variant_new_int32(mission->no_teams));

    mission_dict_array = g_variant_builder_end(mission_builder);

    g_variant_builder_add(builder, "{sv}", "mission", mission_dict_array);
    g_variant_builder_unref(mission_builder);
}

static void create_gameroom_custom_params(s_gr_custom_params *custom_params, GVariantBuilder *builder)
{
    GVariantBuilder *custom_params_builder;
    GVariant *custom_params_dict_array;

    custom_params_builder = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));

    create_gr_sync(&custom_params->base, custom_params_builder);
    g_variant_builder_add(custom_params_builder, "{sv}", "friendly_fire", g_variant_new_int32(custom_params->friendly_fire));
    g_variant_builder_add(custom_params_builder, "{sv}", "enemy_outlines", g_variant_new_int32(custom_params->enemy_outlines));
    g_variant_builder_add(custom_params_builder, "{sv}", "auto_team_balance", g_variant_new_int32(custom_params->auto_team_balance));
    g_variant_builder_add(custom_params_builder, "{sv}", "dead_can_chat", g_variant_new_int32(custom_params->dead_can_chat));
    g_variant_builder_add(custom_params_builder, "{sv}", "join_in_the_process", g_variant_new_int32(custom_params->join_in_the_process));
    g_variant_builder_add(custom_params_builder, "{sv}", "locked_spectator_camera", g_variant_new_int32(custom_params->locked_spectator_camera));

    g_variant_builder_add(custom_params_builder, "{sv}", "preround_time", g_variant_new_int32(custom_params->preround_time));

    g_variant_builder_add(custom_params_builder, "{sv}", "max_players", g_variant_new_int32(custom_params->max_players));
    g_variant_builder_add(custom_params_builder, "{sv}", "round_limit", g_variant_new_int32(custom_params->round_limit));
    g_variant_builder_add(custom_params_builder, "{sv}", "class_restriction", g_variant_new_int32(custom_params->class_restriction));
    g_variant_builder_add(custom_params_builder, "{sv}", "inventory_slot", g_variant_new_double(custom_params->inventory_slot));

    custom_params_dict_array = g_variant_builder_end(custom_params_builder);

    g_variant_builder_add(builder, "{sv}", "custom_params", custom_params_dict_array);
    g_variant_builder_unref(custom_params_builder);
}

static void create_gameroom_core(s_gr_core *core, GVariantBuilder *builder)
{
    GVariantBuilder *core_builder;
    GVariant *core_dict_array;

    core_builder = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));

    create_gr_sync(&core->base, core_builder);
    g_variant_builder_add(core_builder, "{sv}", "room_name", g_variant_new_string(core->room_name));
    g_variant_builder_add(core_builder, "{sv}", "teams_switched", g_variant_new_int32(core->teams_switched));
    g_variant_builder_add(core_builder, "{sv}", "private", g_variant_new_int32(core->private));
    g_variant_builder_add(core_builder, "{sv}", "can_start", g_variant_new_int32(core->can_start));
    g_variant_builder_add(core_builder, "{sv}", "team_balanced", g_variant_new_int32(core->team_balanced));
    g_variant_builder_add(core_builder, "{sv}", "min_ready_players", g_variant_new_int32(core->min_ready_players));
    g_variant_builder_add(core_builder, "{sv}", "num_players", g_variant_new_int32(core->num_players));

    core_dict_array = g_variant_builder_end(core_builder);

    g_variant_builder_add(builder, "{sv}", "core", core_dict_array);
    g_variant_builder_unref(core_builder);
}

static void rlist_to_array(struct gameroom *rl, GVariantBuilder *builder)
{
    GVariantBuilder *item_builder;

    item_builder = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));

    g_variant_builder_add(item_builder, "{sv}", "room_id", g_variant_new_string(rl->room_id));
    g_variant_builder_add(item_builder, "{sv}", "room_type", g_variant_new_string(
                                                                        rl->room_type == ROOM_PVE_PRIVATE ? "PVE_PRIVATE":
                                                                        rl->room_type == ROOM_PVP_PUBLIC ? "PVP_PUBLIC":
                                                                        rl->room_type == ROOM_PVP_CLANWAR ? "PVP_CLANWAR":
                                                                        rl->room_type == ROOM_PVP_QUICKPLAY ? "PVP_QUICKPLAY":
                                                                        rl->room_type == ROOM_PVE_QUICKPLAY ? "PVE_QUICKPLAY": "PVP_RATING"));

    create_gameroom_core(&rl->core, item_builder);
    create_gameroom_custom_params(&rl->custom_params, item_builder);
    create_gameroom_mission(&rl->mission, item_builder);
    create_gameroom_session(&rl->session, item_builder);
    create_gameroom_room_master(&rl->room_master, item_builder);
    create_gameroom_regions(&rl->regions, item_builder);
    create_gameroom_auto_start(&rl->auto_start, item_builder);

    if(rl->clan_war.clan_1 != NULL || rl->clan_war.clan_2 != NULL)
        create_gameroom_clan_war(&rl->clan_war, item_builder);

    g_variant_builder_add(builder, "a{sv}", item_builder);
    g_variant_builder_unref(item_builder);
}

void cmd_room_get_dbus_cb(struct list *room_list, void *args)
{
    struct cb_args *a = (struct cb_args *) args;

    GVariantBuilder *builder;
    GVariant *rooms_dict_array;

    builder = g_variant_builder_new(G_VARIANT_TYPE("aa{sv}"));

    if (room_list == NULL)
    {
        rooms_dict_array = g_variant_builder_end(builder);
    }
    else
    {
        list_foreach(room_list, (f_list_callback) rlist_to_array, builder);
        rooms_dict_array = g_variant_builder_end(builder);
    }

    warfacebot_complete_room_get(a->object, a->invocation, rooms_dict_array);
}

/*
** DBus method call: "RoomGet"
*/
gboolean on_handle_room_get(Warfacebot *object,
                                 GDBusMethodInvocation *invocation,
                                 const gchar *arg_channel,
                                 gint arg_room_type)
{
    struct cb_args *a = g_new0(struct cb_args, 1);

    a->object = object;
    a->invocation = invocation;

    xmpp_iq_gameroom_get(arg_channel, arg_room_type, cmd_room_get_dbus_cb, a);

    return TRUE;
}
