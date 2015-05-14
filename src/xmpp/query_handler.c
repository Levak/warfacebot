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

#include <wb_xmpp.h>

#include <string.h>

struct query_handler {
    f_query_callback callback;
    char query[32];
};

#define QUERY_HDLR_MAX 16
struct query_handler query_handlers[QUERY_HDLR_MAX] = { { 0 } };

void qh_register(const char *query, f_query_callback callback)
{
    int i = 0;
    for (; i < QUERY_HDLR_MAX; ++i)
        if (!query_handlers[i].callback)
            break;

    if (!query_handlers[i].callback)
    {
        query_handlers[i].callback = callback;
        strncpy(query_handlers[i].query, query, sizeof (query_handlers[i].query));
    }
}

int qh_handle(const char *query, const char *msg_id, const char *msg)
{
    if (!query || !*query)
        return 0;

    int i = 0;
    for (; i < QUERY_HDLR_MAX; ++i)
    {
        if (strncmp(query_handlers[i].query, query, sizeof(query_handlers[i].query)) == 0)
        {
            query_handlers[i].callback(msg_id, msg);
            return 1;
        }
    }

    return 0;
}
