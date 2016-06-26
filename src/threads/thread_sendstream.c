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
#include <wb_stream.h>
#include <wb_threads.h>

#include <semaphore.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define SEND_MSG_MAX 16
static sem_t _sem_send_msgs_empty;
static sem_t _sem_send_msgs_full;
static char *send_msgs[SEND_MSG_MAX] = { 0 };

void thread_sendstream_post_new_msg(char *msg)
{
    static int pos = 0;

    assert(msg != NULL);

    sem_wait(&_sem_send_msgs_empty);

    send_msgs[pos] = msg;

    ++pos;
    if (pos >= SEND_MSG_MAX)
        pos = 0;

    sem_post(&_sem_send_msgs_full);
}

char *thread_sendstream_get_next_msg(void)
{
    static int pos = 0;
    char *msg = NULL;

    sem_wait(&_sem_send_msgs_full);

    msg = send_msgs[pos];
    send_msgs[pos] = NULL;

    ++pos;
    if (pos >= SEND_MSG_MAX)
        pos = 0;

    sem_post(&_sem_send_msgs_empty);

    return msg;
}

void thread_sendstream_init(void)
{
    if (sem_init(&_sem_send_msgs_empty, 0, SEND_MSG_MAX) != 0)
        perror("semaphore init failed");
    if (sem_init(&_sem_send_msgs_full, 0, 0) != 0)
        perror("semaphore init failed");
}

void *thread_sendstream(void *vargs)
{
    thread_register_sigint_handler();

    do {
        char *msg = thread_sendstream_get_next_msg();

        if (session.active)
            assert(msg != NULL);

        if (msg == NULL)
            break;

        stream_send_msg(session.wfs, msg);
        stream_flush(session.wfs);

        free(msg);
    } while (session.active);

    /* Destroy remaining messages */
    for (unsigned int i = 0; i < SEND_MSG_MAX; ++i)
    {
        free(send_msgs[i]);
        send_msgs[i] = NULL;
    }

    sem_destroy(&_sem_send_msgs_empty);
    sem_destroy(&_sem_send_msgs_full);

    return thread_close();
}
