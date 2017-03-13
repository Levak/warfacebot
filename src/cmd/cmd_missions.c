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
#include <wb_session.h>
#include <wb_list.h>
#include <wb_mission.h>
#include <wb_xmpp_wf.h>
#include <wb_cmd.h>
#include <wb_log.h>
#include <wb_lang.h>

#include <string.h>

struct cb_args
{
    f_cmd_missions_cb cb;
    void *args;
};

static void cbm(struct mission *m, void *args)
{
    if(m->setting == NULL)
        return;

    struct cb_args *a = (struct cb_args *) args;

    char *setting = strdup(m->setting);
    char *p = strstr(setting, "/");

    if (p != NULL)
        *p = 0;

    a->cb(m->type, setting, m, a->args);

    free(setting);
}

void cmd_missions(f_cmd_missions_cb cb, void *args)
{
    if (cb == NULL)
        return;

    struct cb_args a = { cb, args };

    if (session.wf.missions.list == NULL)
        return;

    list_foreach(session.wf.missions.list, (f_list_callback) cbm, &a);
}

void cmd_missions_whisper_cb(const char *type,
                             const char *setting,
                             struct mission *m,
                             void *args)
{
    if(m->crown_time_gold == 0)
        return;

    struct whisper_cb_args *a = (struct whisper_cb_args *) args;

    if (a != NULL && a->nick_to != NULL && a->jid_to != NULL)
    {
        char *answer =
            LANG_FMT(whisper_missions_line,
                     m->name,
                     setting,
                     m->crown_time_gold / 60,
                     m->crown_perf_gold / 1000);

        xmpp_send_message(a->nick_to, a->jid_to, answer);

        free(answer);
    }
}

void cmd_missions_console_cb(const char *type,
                             const char *setting,
                             struct mission *m,
                             void *args)
{
    if (m->crown_time_gold != 0)
    {
        xprintf(" - %20s %-10s %s: %2i:%02i %s: %i",
                m->name,
                setting,
                LANG(console_missions_time),
                m->crown_time_gold / 60,
                m->crown_time_gold % 60,
                LANG(console_missions_score),
                m->crown_perf_gold);
    }
    else
    {
        xprintf(" - %20s %-9s",
                m->name,
                setting);
    }
}

void cmd_missions_wrapper(void)
{
    cmd_missions(cmd_missions_console_cb, NULL);
}
