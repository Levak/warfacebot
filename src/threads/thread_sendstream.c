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
#include <wb_stream.h>
#include <wb_threads.h>

#include <semaphore.h>
#include <pthread.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define SEND_MSG_MAX 16
static sem_t _sem_send_msgs_empty;
static sem_t _sem_send_msgs_full;
static char *send_msgs[SEND_MSG_MAX] = { 0 };
static pthread_mutex_t _lock_send_id;

void thread_sendstream_post_new_msg(char *msg)
{
    static int pos = 0;

    assert(msg != NULL);

    sem_wait(&_sem_send_msgs_empty);

    pthread_mutex_lock(&_lock_send_id);
    send_msgs[pos] = msg;

    ++pos;
    if (pos >= SEND_MSG_MAX)
        pos = 0;
    pthread_mutex_unlock(&_lock_send_id);

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

    assert(msg != NULL);

    return msg;
}

void thread_sendstream_init(void)
{
    if (sem_init(&_sem_send_msgs_empty, 0, SEND_MSG_MAX) != 0)
        perror("semaphore init failed");
    if (sem_init(&_sem_send_msgs_full, 0, 0) != 0)
        perror("semaphore init failed");
    if (pthread_mutex_init(&_lock_send_id, NULL) != 0)
        perror("mutex init failed");
}

static void thread_sendstream_close(void *vargs)
{
    struct thread *t = (struct thread *) vargs;

    /* Destroy remaining messages */
    for (unsigned int i = 0; i < SEND_MSG_MAX; ++i)
    {
        free(send_msgs[i]);
        send_msgs[i] = NULL;
    }

    sem_destroy(&_sem_send_msgs_empty);
    sem_destroy(&_sem_send_msgs_full);
    pthread_mutex_destroy(&_lock_send_id);
}

void *thread_sendstream(void *vargs)
{
    struct thread *t = (struct thread *) vargs;

    pthread_cleanup_push(thread_sendstream_close, t);

    while (session.state != STATE_DEAD)
    {
        char *msg = thread_sendstream_get_next_msg();

        stream_send_msg(session.wfs, msg);
        stream_flush(session.wfs);

        free(msg);
    }

    pthread_cleanup_pop(1);
    return thread_close(t);
}
