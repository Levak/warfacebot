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
#include <wb_threads.h>
#include <wb_dbus.h>
#include <wb_log.h>

#include <signal.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#ifdef __MINGW32__
# include <windows.h>
# define sleep(x) Sleep(x)
#endif

static void sigint_handler(int signum)
{
    session.active = 0;

#ifdef DBUS_API
    dbus_api_quit(0);
#endif

    pthread_exit(NULL);
}

void thread_register_sigint_handler(void)
{
    signal(SIGINT, sigint_handler);
}

static int thread_cmp(pthread_t *t, const char *name)
{
    char own_name[THREAD_NAME_MAX];

    pthread_getname_np(*t, own_name, THREAD_NAME_MAX);

    return strncmp(own_name, name, THREAD_NAME_MAX);
}

static void thread_free(pthread_t *t)
{
    free(t);
}

static void thread_kill(pthread_t *t, void *args)
{
    pthread_kill(*t, SIGINT);
}

static struct list *thread_list;

#define CREATE_THREAD(Name) do {                                        \
        pthread_t *t = calloc(1, sizeof (pthread_t));                   \
        thread_ ## Name ## _init();                                     \
        if (pthread_create(t, NULL, &(thread_ ## Name), NULL) == -1)    \
            perror("pthread_create " #Name);                            \
        else                                                            \
        {                                                               \
            pthread_setname_np(*t, #Name);                              \
            pthread_detach(*t);                                         \
            list_add(thread_list, t);                                   \
        }                                                               \
    } while (0)

void threads_init(void)
{
    thread_list = list_new((f_list_cmp) thread_cmp,
                           (f_list_free) thread_free);

    CREATE_THREAD(readstream);
    CREATE_THREAD(sendstream);
    CREATE_THREAD(dispatch);

#if ! defined DBUS_API || defined DEBUG
    CREATE_THREAD(readline);
#endif

    CREATE_THREAD(ping);
}

void threads_run(void)
{
#ifdef DBUS_API
    dbus_api_enter();
#else
    while (session.active)
        sleep(1);
#endif

#ifdef DBUS_API
    dbus_api_quit(0);
#endif

    xprintf("Closed idle\n");

    list_free(thread_list);
}

void *thread_close(void)
{
    char own_name[THREAD_NAME_MAX];

    pthread_getname_np(pthread_self(), own_name, THREAD_NAME_MAX);

    xprintf("Closed %s\n", own_name);

    session.active = 0;

    list_foreach(thread_list,
                 (f_list_callback) thread_kill,
                 NULL);

    pthread_exit(NULL);
}
