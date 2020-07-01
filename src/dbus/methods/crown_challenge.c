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
#include <wb_mission.h>
#include <wb_session.h>
#include <wb_list.h>

static void mlist_to_array(struct mission *m, GVariantBuilder *builder)
{
    if (m->crown_time_gold == 0)
        return;

    GVariantBuilder *mission_builder;

    mission_builder = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));

    g_variant_builder_add(mission_builder, "{sv}", "mission_key", g_variant_new_string(m->mission_key));
    g_variant_builder_add(mission_builder, "{sv}", "no_team", g_variant_new_int32(m->no_team));
    g_variant_builder_add(mission_builder, "{sv}", "original_name", g_variant_new_string(m->original_name));
    g_variant_builder_add(mission_builder, "{sv}", "name", g_variant_new_string(m->name));
    g_variant_builder_add(mission_builder, "{sv}", "setting", g_variant_new_string(m->setting));
    g_variant_builder_add(mission_builder, "{sv}", "mode", g_variant_new_string(m->mode));
    g_variant_builder_add(mission_builder, "{sv}", "mode_name", g_variant_new_string(m->mode_name));
    g_variant_builder_add(mission_builder, "{sv}", "mode_icon", g_variant_new_string(m->mode_icon));
    g_variant_builder_add(mission_builder, "{sv}", "description", g_variant_new_string(m->description));
    g_variant_builder_add(mission_builder, "{sv}", "image", g_variant_new_string(m->image));
    g_variant_builder_add(mission_builder, "{sv}", "difficulty", g_variant_new_string(m->difficulty));
    g_variant_builder_add(mission_builder, "{sv}", "type", g_variant_new_string(m->type));
    g_variant_builder_add(mission_builder, "{sv}", "time_of_day", g_variant_new_string(m->time_of_day));

    g_variant_builder_add(mission_builder, "{sv}", "crown_reward_bronze", g_variant_new_int32(m->crown_reward_bronze));
    g_variant_builder_add(mission_builder, "{sv}", "crown_reward_silver", g_variant_new_int32(m->crown_reward_silver));
    g_variant_builder_add(mission_builder, "{sv}", "crown_reward_gold", g_variant_new_int32(m->crown_reward_gold));

    g_variant_builder_add(mission_builder, "{sv}", "crown_time_bronze", g_variant_new_int32(m->crown_time_bronze));
    g_variant_builder_add(mission_builder, "{sv}", "crown_time_silver", g_variant_new_int32(m->crown_time_silver));
    g_variant_builder_add(mission_builder, "{sv}", "crown_time_gold", g_variant_new_int32(m->crown_time_gold));

    g_variant_builder_add(mission_builder, "{sv}", "crown_perf_bronze", g_variant_new_int32(m->crown_perf_bronze));
    g_variant_builder_add(mission_builder, "{sv}", "crown_perf_silver", g_variant_new_int32(m->crown_perf_silver));
    g_variant_builder_add(mission_builder, "{sv}", "crown_perf_gold", g_variant_new_int32(m->crown_perf_gold));

    g_variant_builder_add(builder, "a{sv}", mission_builder);
    g_variant_builder_unref(mission_builder);
}

static GVariant *crown_challenge_dict_array = NULL;
static gboolean invalidated = TRUE;

/*
** Update the cached crown challenge for DBus API.
*/
void dbus_api_update_crown_challenge(void)
{
    invalidated = TRUE;
}

/*
** DBus method call: "CrownChallenge"
*/
gboolean on_handle_crown_challenge(Warfacebot *object,
                                   GDBusMethodInvocation *invocation)
{
    if (invalidated)
    {
        struct list *ml = session.wf.missions.list;

        GVariantBuilder *missions_dict_array_builder, *crown_challenge_builder;
        GVariant *missions_dict_array;

        if (crown_challenge_dict_array != NULL)
            g_variant_unref(crown_challenge_dict_array);

        missions_dict_array_builder = g_variant_builder_new(G_VARIANT_TYPE("aa{sv}"));
        list_foreach(ml, (f_list_callback) mlist_to_array, missions_dict_array_builder);
        missions_dict_array = g_variant_builder_end(missions_dict_array_builder);

        crown_challenge_builder = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));

        g_variant_builder_add(crown_challenge_builder, "{sv}", "hash", g_variant_new_int32(session.wf.missions.hash));
        g_variant_builder_add(crown_challenge_builder, "{sv}", "content_hash", g_variant_new_int32(session.wf.missions.content_hash));
        g_variant_builder_add(crown_challenge_builder, "{sv}", "missions", missions_dict_array);

        crown_challenge_dict_array = g_variant_builder_end(crown_challenge_builder);
        g_variant_ref(crown_challenge_dict_array);

        invalidated = FALSE;
    }

    warfacebot_complete_crown_challenge(object, invocation, crown_challenge_dict_array);

    return TRUE;
}
