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

#include <stdlib.h>
#include <string.h>

#include <wb_tools.h>

char *get_msg_id(const char *msg)
{
    char *msg_id = NULL;
    char *first = get_info(msg, "<", ">", NULL);

    if (first)
    {
        msg_id = get_info(first, "id='", "'", NULL);
        free(first);
    }

    return msg_id;
}

char *get_query_tag_name(const char *msg)
{
    char *stanza = NULL;
    char *iq_pos = strstr(msg, "<iq");

    if (iq_pos)
    {
        iq_pos += sizeof ("<iq") - 1;
        char *query_pos = strstr(iq_pos, "<query");
        if (query_pos)
        {
            query_pos += sizeof ("<query") - 1;
            char *data_pos = strstr(query_pos, "<data");
            if (data_pos)
            {
                data_pos += sizeof ("<data") - 1;
                stanza = get_info(data_pos, "query_name='", "'", NULL);
            }
            else
                stanza = get_info_first(query_pos, "<", "/> ", NULL);
        }
        else
            stanza = get_info_first(iq_pos, "<", "/> ", NULL);
    }
    else
        stanza = get_info_first(msg, "<", "/> ", NULL);

    if (stanza && !*stanza)
    {
        free(stanza);
        stanza = NULL;
    }

    return stanza;
}

inline int xmpp_is_error(const char *msg)
{
    return (msg == NULL ||
            strstr(msg, "<error") != NULL);
}
