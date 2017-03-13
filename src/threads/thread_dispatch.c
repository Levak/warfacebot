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

#include <wb_session.h>
#include <wb_xmpp.h>
#include <wb_threads.h>
#include <wb_log.h>

#include <assert.h>

void thread_dispatch_init(void)
{
    /* Nothing to do */
}

static void thread_dispatch_close(void *vargs)
{
    /* Nothing to do */
}

void *thread_dispatch(void *vargs)
{
    struct thread *t = (struct thread *) vargs;

    pthread_cleanup_push(thread_dispatch_close, t);

    while (session.state != STATE_DEAD)
    {
        char *msg = thread_readstream_get_next_msg();

        if (session.state != STATE_DEAD)
            assert(msg != NULL);

        if (msg == NULL)
            break;

        char *msg_id = get_msg_id(msg);

        enum xmpp_msg_type type = get_msg_type(msg);

        /* If we expect an answer from that ID */
        if (msg_id != NULL && idh_handle(msg_id, msg, type))
        {
            /* Good, we handled it */
        }
        /* If someone thinks we expected an answer */
        else if (type & (XMPP_TYPE_ERROR | XMPP_TYPE_RESULT))
        {
#ifdef DEBUG
            if (msg_id != NULL)
            {
                /* Unhandled stanza */
                eprintf("FIXME - Unhandled id: %s\n%s\n", msg_id, msg);
            }
#endif
        }
        /* If it wasn't handled and it's not a result */
        else
        {
            char *stanza = get_query_tag_name(msg);

            if (stanza == NULL)
            {
#ifdef DEBUG
                eprintf("FIXME - Unhandled msg:\n%s\n", msg);
#endif
            }
            /* Look if tagname is registered */
            else if (qh_handle(stanza, msg_id, msg))
            {
                /* Good, we handled it */
            }
            else
            {
#ifdef DEBUG
                /* Unhandled stanza */
                eprintf("FIXME - Unhandled query: %s\n%s\n", stanza, msg);
#endif
            }

            free(stanza);
        }

        free(msg);
        free(msg_id);

    }

    pthread_cleanup_pop(1);
    return thread_close(t);
}
