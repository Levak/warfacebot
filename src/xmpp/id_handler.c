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

#include <wb_xmpp.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <pthread.h>

struct id_handler {
    f_id_callback callback;
    void *args;
    char permanent;
    time_t timestamp;
    t_uid id;
};

static struct id_handler *id_handlers = NULL;
static size_t id_handlers_size = 64;

static pthread_mutex_t _lock_id;
static pthread_mutex_t _lock_handlers;

static unsigned int _id = 0;

static void idh_free(void)
{
    pthread_mutex_destroy(&_lock_id);
    pthread_mutex_destroy(&_lock_handlers);

    free(id_handlers);
}

void idh_init(void)
{
    atexit(idh_free);

    if (pthread_mutex_init(&_lock_id, NULL) != 0)
        perror("mutex init failed");
    if (pthread_mutex_init(&_lock_handlers, NULL) != 0)
        perror("mutex init failed");

    id_handlers = calloc(sizeof (struct id_handler), id_handlers_size);
}

void idh_register(const t_uid *id, int permanent,
                  f_id_callback callback, void *args)
{
    if (callback == NULL)
        return;

    pthread_mutex_lock(&_lock_handlers);

    size_t i = 0;
    for (; i < id_handlers_size; ++i)
        if (!id_handlers[i].id.uid[0])
            break;

    if (i >= id_handlers_size)
    {
        id_handlers_size *= 2;
        id_handlers = realloc(
            id_handlers,
            sizeof (struct id_handler) * id_handlers_size);
        memset(&id_handlers[i],
               0,
               sizeof (struct id_handler) * (id_handlers_size - i));
    }

    id_handlers[i].id = *id;
    id_handlers[i].callback = callback;
    id_handlers[i].args = args;
    id_handlers[i].permanent = permanent;
    id_handlers[i].timestamp = time(NULL);

    pthread_mutex_unlock(&_lock_handlers);
}

int idh_handle(const char *msg_id, const char *msg, enum xmpp_msg_type type)
{
    if (!msg_id)
        return 0;

    time_t t = time(NULL);

    pthread_mutex_lock(&_lock_handlers);

    size_t i = 0;
    for (; i < id_handlers_size; ++i)
    {
        /* handler id matches */
        if (msg_id != NULL
            && msg_id[0] != 0
            && 0 == strncmp(id_handlers[i].id.uid,
                    msg_id,
                    sizeof (id_handlers[i].id.uid)))
        {
            f_id_callback callback = id_handlers[i].callback;
            void *args = id_handlers[i].args;

            if (!id_handlers[i].permanent)
            {
                id_handlers[i].id.uid[0] = 0;
            }

            pthread_mutex_unlock(&_lock_handlers);

            callback(msg, type, args);

            return 1;
        }

        /* handler is sitting here for too long */
        if (id_handlers[i].id.uid[0] != 0
            && id_handlers[i].timestamp + 120 < t
            && !id_handlers[i].permanent)
        {
            f_id_callback callback = id_handlers[i].callback;
            void *args = id_handlers[i].args;

            id_handlers[i].id.uid[0] = 0;

            pthread_mutex_unlock(&_lock_handlers);

            callback(NULL, XMPP_TYPE_ERROR, args);

            pthread_mutex_lock(&_lock_handlers);
        }
    }

    pthread_mutex_unlock(&_lock_handlers);

    return 0;
}

void idh_generate_unique_id(t_uid *id)
{
    pthread_mutex_lock(&_lock_id);
    ++_id;
    pthread_mutex_unlock(&_lock_id);

    sprintf((char *)id->uid, XMPP_ID, _id);
}
