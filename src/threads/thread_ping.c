/**
 * WarfaceBot, a blind XMPP client for Warface (FPS)
 * Copyright (C) 2015, 2016 Levak Borok <levak92@gmail.com>
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

#include <wb_session.h>
#include <wb_xmpp.h>
#include <wb_threads.h>
#include <wb_log.h>
#include <wb_cvar.h>

void thread_ping_init(void)
{
    /* Nothing to do */
}

static void thread_ping_close(void *vargs)
{
    /* Nothing to do */
}

void *thread_ping(void *vargs)
{
    struct thread *t = (struct thread *) vargs;

    pthread_cleanup_push(thread_ping_close, t);

    int previous_ping = 0;

    while (session.state != STATE_DEAD)
    {
        if (session.xmpp.last_query
            + (cvar.wb_ping_count_is_over
               * cvar.wb_ping_unit) < time(NULL))
        {
            xprintf("it's over.\n\n");
            break;
        }
        else if (session.xmpp.last_query
                 + (cvar.wb_ping_count_is_stall
                    * cvar.wb_ping_unit) < time(NULL))
        {
            xprintf("Stalling life... ");
            xmpp_iq_ping();
            previous_ping = 1;
        }
        else if (previous_ping)
        {
            xprintf("still there!\n");
            previous_ping = 0;
        }

        if (session.online.last_status_update
            + (cvar.wb_ping_count_is_outdated
               * cvar.wb_ping_unit) < time(NULL))
        {
            xmpp_iq_player_status(session.online.status);
        }

        sleep(cvar.wb_ping_unit);
    }

    pthread_cleanup_pop(1);
    return thread_close(t);
}
