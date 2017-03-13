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

#include <wb_masterserver.h>
#include <wb_list.h>
#include <wb_session.h>
#include <wb_xmpp_wf.h>
#include <wb_dbus.h>

#include <stdlib.h>
#include <string.h>

static int masterserver_cmp(const struct masterserver *ms,
                            const char *resource)
{
    return strcmp(ms->resource, resource);
}

static int masterserver_cmp_type(const struct masterserver *ms,
                                 const char *channel_type)
{
    return strcmp(ms->channel, channel_type);
}

static void masterserver_free(struct masterserver *ms)
{
    free(ms->resource);
    free(ms->channel);
    free(ms->rank_group);
    free(ms->bootstrap);

    free(ms);
}

struct masterserver *masterserver_list_get(const char *resource)
{
    if (session.online.masterservers == NULL || resource == NULL)
        return NULL;

    return list_get(session.online.masterservers, resource);
}

struct masterserver *masterserver_list_get_by_type(const char *type)
{
    if (session.online.masterservers == NULL || type == NULL)
        return NULL;

    return list_get_by(session.online.masterservers,
                       type,
                       (f_list_cmp) masterserver_cmp_type);
}

struct list *masterserver_list_new(void)
{
    return list_new((f_list_cmp) masterserver_cmp,
                    (f_list_free) masterserver_free);
}

struct cb_args
{
    f_msl_update_cb fun;
    void *args;
};

static void cb(struct list *masterservers, void *args)
{
    struct cb_args *a = (struct cb_args *) args;

    masterserver_list_free();

    session.online.masterservers = masterservers;

    if (a->fun != NULL)
        a->fun(a->args);

    free(a);
}

void masterserver_list_update(f_msl_update_cb fun, void *args)
{
    struct cb_args *a = calloc(1, sizeof (struct cb_args));
    a->fun = fun;
    a->args = args;

    xmpp_iq_get_master_servers(cb, a);
}

void masterserver_list_init(struct list *masterservers)
{
    session.online.masterservers = masterservers;
}

void masterserver_list_free(void)
{
    if (session.online.masterservers != NULL)
        list_free(session.online.masterservers);

    session.online.masterservers = NULL;
}
