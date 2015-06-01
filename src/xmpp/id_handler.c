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

#include <stdio.h>
#include <string.h>

struct id_handler {
    f_id_callback callback;
    void *args;
    char permanent;
    t_uid id;
};

#define ID_HDLR_MAX 16
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

    if (!id_handlers[i].id.uid[0])
    {
        id_handlers[i].id = *id;
        id_handlers[i].callback = callback;
        id_handlers[i].args = args;
        id_handlers[i].permanent = permanent;
    }
}

int idh_handle(const char *msg_id, const char *msg)
{
    if (!msg_id)
        return 0;

    int i = 0;
    for (; i < ID_HDLR_MAX; ++i)
    {
        if (strncmp(id_handlers[i].id.uid, msg_id, sizeof (id_handlers[i].id.uid)) == 0)
        {
            if (!id_handlers[i].permanent)
                id_handlers[i].id.uid[0] = 0;
            id_handlers[i].callback(msg, id_handlers[i].args);
            return 1;
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
