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

struct cb_args
{
    const char *nick_to;
    const char *jid_to;
};

static void cbm(struct mission *m, void *args)
{
    struct cb_args *a = (struct cb_args *) args;

    if (a->nick_to != NULL && a->jid_to != NULL)
    {
        char *answer;
        FORMAT(answer, "mission %s %i %i",
               m->type,
               m->crown_time_gold,
               m->crown_perf_gold);

        xmpp_send_message(session.wfs, session.nickname, session.jid,
                          a->nick_to, a->jid_to,
                          answer, NULL);

        free(answer);
    }
    else
    {
        printf("mission %s %i %i",
               m->type,
               m->crown_time_gold,
               m->crown_perf_gold);
    }
}

void cmd_missions(const char *nick_to, const char *jid_to)
{
    struct cb_args a = { nick_to, jid_to };

    list_foreach(session.missions, (f_list_callback) cbm, &a);
}
