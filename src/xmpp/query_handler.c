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

#include <string.h>
#include <assert.h>
#include <time.h>

struct query_handler {
    f_query_callback callback;
    void *args;
    char permanent;
    time_t timestamp;
    char query[32];
};

#define QUERY_HDLR_MAX 64
struct query_handler query_handlers[QUERY_HDLR_MAX] = { { 0 } };

void qh_register(const char *query, int permanent,
                 f_query_callback callback, void *args)
{
    if (callback == NULL)
        return;

    int i = 0;
    for (; i < QUERY_HDLR_MAX; ++i)
        if (!query_handlers[i].query[0])
            break;

    assert(i < QUERY_HDLR_MAX && "QUERY HDLR OVERFLOW\n");

    query_handlers[i].callback = callback;
    query_handlers[i].args = args;
    query_handlers[i].permanent = permanent;
    query_handlers[i].timestamp = time(NULL);

    strncpy(query_handlers[i].query, query, sizeof (query_handlers[i].query));
}

void qh_remove(const char *query)
{
    if (!query || !*query)
        return;

    int i = 0;
    for (; i < QUERY_HDLR_MAX; ++i)
    {
        /* handler id matches */
        if (strncmp(query_handlers[i].query, query, sizeof(query_handlers[i].query)) == 0)
        {
            query_handlers[i].query[0] = 0;
        }
    }
}

int qh_handle(const char *query, const char *msg_id, const char *msg)
{
    if (!query || !*query)
        return 0;

    time_t t = time(NULL);

    int i = 0;
    for (; i < QUERY_HDLR_MAX; ++i)
    {
        /* handler id matches */
        if (strncmp(query_handlers[i].query, query, sizeof(query_handlers[i].query)) == 0)
        {
            if (!query_handlers[i].permanent)
                query_handlers[i].query[0] = 0;
            query_handlers[i].callback(msg_id, msg, query_handlers[i].args);
            return 1;
        }

        /* handler is sitting here for too long */
        if (query_handlers[i].query[0] != 0
            && query_handlers[i].timestamp + 120 < t
            && !query_handlers[i].permanent)
        {
            query_handlers[i].query[0] = 0;
            query_handlers[i].callback(NULL, NULL, query_handlers[i].args);
        }
    }

    return 0;
}
