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

#include <wb_tools.h>
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>
#include <wb_cmd.h>
#include <wb_masterserver.h>
#include <wb_log.h>
#include <wb_lang.h>

struct cb_args
{
    f_cmd_stats_cb cb;
    void *args;
};

static void _masterserver_cb(struct masterserver *ms, void *args)
{
    struct cb_args *a = (struct cb_args *) args;

    if (ms->resource != NULL)
        a->cb(ms->resource, ms->online, a->args);
}

static void cmd_stats_cb(struct list *masterserver,
                         void *args)
{
    struct cb_args *a = (struct cb_args *) args;

    if (a->cb)
    {
        list_foreach(masterserver,
                     (f_list_callback) _masterserver_cb,
                     args);

        a->cb(NULL, 0, a->args);
    }

    list_free(masterserver);

    free(a);
}

void cmd_stats(f_cmd_stats_cb cb, void *args)
{
    struct cb_args *a = calloc(1, sizeof (struct cb_args));

    a->cb = cb;
    a->args = args;

    xmpp_iq_get_master_servers(cmd_stats_cb, a);
}

void cmd_stats_console_cb(const char *resource, int online, void *args)
{
    int *p_stats_online_total = (int *) args;

    if (resource != NULL)
    {
        xprintf(" - %15s %4i", resource, online);
        *p_stats_online_total += online;
    }
    else
    {
        xprintf("%s: \033[1;32m%i\033[0m",
                LANG(console_stats_total),
                *p_stats_online_total);

        free(p_stats_online_total);
    }
}

void cmd_stats_wrapper(void)
{
    int *p_stats_online_total = calloc(1, sizeof (int));

    cmd_stats(cmd_stats_console_cb, p_stats_online_total);
}
