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
#include <wb_xmpp_wf.h>
#include <wb_status.h>

#include <semaphore.h>
#include <pthread.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define RECV_MSG_MAX 16
static sem_t _sem_recv_msgs_empty;
static sem_t _sem_recv_msgs_full;
static char *recv_msgs[RECV_MSG_MAX] = { 0 };
static pthread_mutex_t _lock_read_id;

/* Shared with xmpp_starttls.c */
pthread_mutex_t _lock_readstream;
pthread_cond_t _cond_readstream;

void thread_readstream_post_new_msg(char *msg)
{
    static int pos = 0;

    assert(msg != NULL);

    sem_wait(&_sem_recv_msgs_empty);

    recv_msgs[pos] = msg;

    ++pos;
    if (pos >= RECV_MSG_MAX)
        pos = 0;

    sem_post(&_sem_recv_msgs_full);
}

char *thread_readstream_get_next_msg(void)
{
    static int pos = 0;
    char *msg = NULL;

    sem_wait(&_sem_recv_msgs_full);

    pthread_mutex_lock(&_lock_read_id);
    msg = recv_msgs[pos];
    recv_msgs[pos] = NULL;

    ++pos;
    if (pos >= RECV_MSG_MAX)
        pos = 0;
    pthread_mutex_unlock(&_lock_read_id);

    sem_post(&_sem_recv_msgs_empty);

    return msg;
}

void thread_readstream_init(void)
{
    if (sem_init(&_sem_recv_msgs_empty, 0, RECV_MSG_MAX) != 0)
        perror("semaphore init failed");
    if (sem_init(&_sem_recv_msgs_full, 0, 0) != 0)
        perror("semaphore init failed");
    if (pthread_mutex_init(&_lock_readstream, NULL) != 0)
        perror("mutex init failed");
    if (pthread_cond_init(&_cond_readstream, NULL) != 0)
        perror("cond init failed");
    if (pthread_mutex_init(&_lock_read_id, NULL) != 0)
        perror("mutex init failed");
}

static void thread_readstream_close(void *vargs)
{
    /* Destroy remaining messages */
    for (unsigned int i = 0; i < RECV_MSG_MAX; ++i)
    {
        free(recv_msgs[i]);
        recv_msgs[i] = NULL;
    }

    sem_destroy(&_sem_recv_msgs_empty);
    sem_destroy(&_sem_recv_msgs_full);
    pthread_mutex_destroy(&_lock_readstream);
    pthread_cond_destroy(&_cond_readstream);
    pthread_mutex_destroy(&_lock_read_id);
}

void *thread_readstream(void *vargs)
{
    struct thread *t = (struct thread *) vargs;

    pthread_cleanup_push(thread_readstream_close, t);

    while (session.state != STATE_DEAD)
    {
        char *msg = NULL;

        if (session.state != STATE_RUN)
        {
            pthread_mutex_lock(&_lock_readstream);

            while (session.state == STATE_TLS_INIT)
                pthread_cond_wait(&_cond_readstream, &_lock_readstream);

            if (session.state == STATE_INIT)
            {
                session.state = STATE_POLL;
                pthread_cond_signal(&_cond_readstream);
            }

            pthread_mutex_unlock(&_lock_readstream);

            msg = stream_read(session.wfs);

            pthread_mutex_lock(&_lock_readstream);

            if (session.state == STATE_POLL)
            {
                session.state = STATE_INIT;
                pthread_cond_signal(&_cond_readstream);
            }

            pthread_mutex_unlock(&_lock_readstream);
        }
        else
        {
            msg = stream_read(session.wfs);
        }

        if (msg == NULL || strlen(msg) <= 0)
        {
            if (session.state != STATE_DEAD)
            {
                status_set(STATUS_LEFT);
            }

            break;
        }

        { /* Replace any " with ' */
            for (char *s = msg; *s; ++s)
                if (*s == '"')
                    *s = '\'';
        }

        thread_readstream_post_new_msg(msg);

        session.xmpp.last_query = time(NULL);
    }

    pthread_cleanup_pop(1);
    return thread_close(t);
}
