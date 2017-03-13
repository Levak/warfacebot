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

#include <wb_mission.h>
#include <wb_list.h>
#include <wb_session.h>
#include <wb_xmpp_wf.h>
#include <wb_dbus.h>

#include <stdlib.h>
#include <string.h>

static int mission_cmp(const struct mission *m, const char *name)
{
    return strcmp(m->name, name);
}

static int mission_cmp_key(const struct mission *m, const char *key)
{
    return strcmp(m->mission_key, key);
}

static void mission_free(struct mission *m)
{
    free(m->mission_key);
    free(m->name);
    free(m->setting);
    free(m->mode);
    free(m->mode_name);
    free(m->mode_icon);
    free(m->description);
    free(m->image);
    free(m->difficulty);
    free(m->type);
    free(m->time_of_day);

    free(m);
}

struct mission *mission_list_get(const char *type)
{
    if (session.wf.missions.list == NULL || type == NULL)
        return NULL;

    return list_get(session.wf.missions.list, type);
}

struct mission *mission_list_get_by_key(const char *key)
{
    if (session.wf.missions.list == NULL || key == NULL)
        return NULL;

    return list_get_by(session.wf.missions.list,
                       key,
                       (f_list_cmp) mission_cmp_key);
}

struct list *mission_list_new(void)
{
    return list_new((f_list_cmp) mission_cmp,
                    (f_list_free) mission_free);
}

struct cb_args
{
    f_ml_update_cb fun;
    void *args;
};

static void cb(struct list *l, int hash, int content_hash, void *args)
{
    struct cb_args *a = (struct cb_args *) args;

    mission_list_free();

    session.wf.missions.list = l;
    session.wf.missions.hash = hash;
    session.wf.missions.content_hash = content_hash;

    if (a->fun != NULL)
        a->fun(a->args);

    free(a);
}

void mission_list_update(f_ml_update_cb fun, void *args)
{
    struct cb_args *a = calloc(1, sizeof (struct cb_args));
    a->fun = fun;
    a->args = args;

    xmpp_iq_missions_get_list(cb, a);

#ifdef DBUS_API
    dbus_api_update_crown_challenge();
#endif
}

void mission_list_init(void)
{
    session.wf.missions.list = NULL;
}

void mission_list_free(void)
{
    if (session.wf.missions.list != NULL)
        list_free(session.wf.missions.list);

    session.wf.missions.list = NULL;
}
