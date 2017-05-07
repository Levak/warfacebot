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
#include <wb_threads.h>
#include <wb_dbus.h>
#include <wb_log.h>

#include <signal.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

/* Main thread reference */
static pthread_t parent;

/**
 * A thread exits its main loop and closes.
 *
 * We need to kill the current session, and tell the main-loop (either DBus or
 * the readline thread) the game is over.
 */
void *thread_close(struct thread *t)
{
    session.state = STATE_DEAD;

    xprintf("Closed %s", t ? t->name : "thread");

#ifdef DBUS_API
    dbus_api_quit(0);
#else /* DBUS_API */
    pthread_kill(parent, SIGINT);
#endif /* DBUS_API */

    pthread_exit(NULL);
}

static int thread_cmp(struct thread *t, const char *name)
{
    return strncmp(t->name, name, THREAD_NAME_MAX);
}

static void thread_free(struct thread *t)
{
    free(t);
}

static void thread_kill(struct thread *t, void *args)
{
    pthread_cancel(t->t);
}

static void thread_join(struct thread *t, void *args)
{
    pthread_join(t->t, NULL);
}

static struct list *thread_list;

#define CREATE_THREAD(Name) do {                                        \
        struct thread *t = calloc(1, sizeof (struct thread));           \
        t->entry = thread_ ## Name;                                     \
        strncpy(t->name, #Name, THREAD_NAME_MAX);                       \
        thread_ ## Name ## _init();                                     \
        list_add(thread_list, t);                                       \
    } while (0)

static void thread_start(struct thread *t, void *args)
{
    if (pthread_create(&t->t, NULL, t->entry, t) == -1)
    {
        list_remove(thread_list, t->name);
        perror("pthread_create");
    }
    else
    {
#ifdef __GLIBC__
        pthread_setname_np(t->t, t->name);
#endif /* __GLIBC__ */
    }
}

/**
 * Initializer child threads.
 *
 * Create and add child threads to a thread list and then call their
 * initialization method.
 */
void threads_init(void)
{
    parent = pthread_self();

    thread_list = list_new((f_list_cmp) thread_cmp,
                           (f_list_free) thread_free);

    CREATE_THREAD(readstream);
    CREATE_THREAD(sendstream);
    CREATE_THREAD(dispatch);

#if defined(DBUS_API) && defined(DEBUG)
    CREATE_THREAD(readline);
#endif /* DBUS_API && DEBUG */

    CREATE_THREAD(ping);
}

/**
 * SIGINT handler for the DBus API.
 *
 * If the program is compiled using the DBus API, there is no easy and safe
 * way to tell it to quit gracefully (since it doesn't have a readline) but
 * sending a Ctrl-C.
 */
static void sigint_handler(int signum)
{
    if (signum == SIGINT)
    {
#if defined(DBUS_API) && ! defined(DEBUG)
        dbus_api_quit(1);
#endif /* DBUS_API && !DEBUG */
    }
}

/**
 * Main thread entry.
 *
 * Create all child threads in bulk then run the main-loop (either DBus or the
 * readline).
 */
void threads_run(const char *exe_path, const char *cmdline)
{
    list_foreach(thread_list,
                 (f_list_callback) thread_start,
                 NULL);

#ifdef DBUS_API
    signal(SIGINT, sigint_handler);
    dbus_api_enter(exe_path, cmdline);
#else /* DBUS_API */
    thread_readline_init();
    thread_readline(NULL);
#endif /* DBUS_API */

    xprintf("Closed idle");
}

/**
 * Gracefully exit all child threads.
 */
void threads_quit(void)
{
    session.state = STATE_DEAD;

    if (thread_list != NULL)
    {
        list_foreach(thread_list,
                     (f_list_callback) thread_kill,
                     NULL);

        list_foreach(thread_list,
                     (f_list_callback) thread_join,
                     NULL);

        list_free(thread_list);
        thread_list = NULL;
    }
}
