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

#include <wb_xmpp.h>

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>

struct id_handler {
    f_id_callback callback;
    void *args;
    char permanent;
    time_t timestamp;
    t_uid id;
};

#define ID_HDLR_MAX 64
struct id_handler id_handlers[ID_HDLR_MAX] = { { 0 } };

void idh_register(const t_uid *id, int permanent,
                  f_id_callback callback, void *args)
{
    if (callback == NULL)
        return;

    int i = 0;
    for (; i < ID_HDLR_MAX; ++i)
        if (!id_handlers[i].id.uid[0])
            break;

    assert(i < ID_HDLR_MAX && "ID HDLR OVERFLOW\n");

    id_handlers[i].id = *id;
    id_handlers[i].callback = callback;
    id_handlers[i].args = args;
    id_handlers[i].permanent = permanent;
    id_handlers[i].timestamp = time(NULL);
}

int idh_handle(const char *msg_id, const char *msg, enum xmpp_msg_type type)
{
    if (!msg_id)
        return 0;

    time_t t = time(NULL);

    int i = 0;
    for (; i < ID_HDLR_MAX; ++i)
    {
        /* handler id matches */
        if (strncmp(id_handlers[i].id.uid, msg_id, sizeof (id_handlers[i].id.uid)) == 0)
        {
            if (!id_handlers[i].permanent)
                id_handlers[i].id.uid[0] = 0;
            id_handlers[i].callback(msg, type, id_handlers[i].args);
            return 1;
        }

        /* handler is sitting here for too long */
        if (id_handlers[i].id.uid[0] != 0
            && id_handlers[i].timestamp + 120 < t
            && !id_handlers[i].permanent)
        {
            id_handlers[i].id.uid[0] = 0;
            id_handlers[i].callback(NULL, XMPP_TYPE_ERROR, id_handlers[i].args);
        }
    }

    return 0;
}

/* TODO: Be thread safe */
void idh_generate_unique_id(t_uid *id)
{
    static unsigned int _id = 0;
    ++_id;

    sprintf((char *)id->uid, XMPP_ID, _id);
}
