/**
 * WarfaceBot, a blind XMPP client for Warface (FPS)
 * Copyright (C) 2015 Levak Borok <levak92@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <wb_tools.h>
#include <wb_session.h>
#include <wb_list.h>
#include <wb_mission.h>
#include <wb_xmpp_wf.h>

#include <stdio.h>
#include <string.h>

struct cb_args
{
    const char *nick_to;
    const char *jid_to;
};

static void cbm(struct mission *m, void *args)
{
    if(m->crown_time_gold == 0)
        return;

    struct cb_args *a = (struct cb_args *) args;

    char *type = strdup(m->type);
    char *p = strstr(type, "mission");

    if (p != NULL)
        *p = 0;

    char *setting = strdup(m->setting);
    p = strstr(setting, "/");

    if (p != NULL)
        *p = 0;

    if (a->nick_to != NULL && a->jid_to != NULL)
    {

        char *answer;
        FORMAT(answer, "%s %s time %imin kill score %ik",
               type, setting,
               m->crown_time_gold / 60,
               m->crown_perf_gold / 1000);

        xmpp_send_message(a->nick_to, a->jid_to, answer);

        free(answer);
    }
    else
    {
        printf("- %s %s\ttime: %i:%i\tcrown: %i\n",
               type,
               setting,
               m->crown_time_gold / 60,
               m->crown_time_gold % 60,
               m->crown_perf_gold);
    }

    free(type);
    free(setting);
}

void cmd_missions(const char *nick_to, const char *jid_to)
{
    struct cb_args a = { nick_to, jid_to };

    list_foreach(session.missions, (f_list_callback) cbm, &a);
}
