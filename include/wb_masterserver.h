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

#ifndef WB_MASTERSERVER_H
# define WB_MASTERSERVER_H

# include <wb_list.h>

struct masterserver
{
    unsigned int server_id;
    unsigned int online;
    unsigned int min_rank;
    unsigned int max_rank;
    char *resource;
    char *channel;
    char *rank_group;
    char *bootstrap;
    float load;
};

struct masterserver *masterserver_list_get(const char *resource);
struct masterserver *masterserver_list_get_by_type(const char *type);

typedef void (*f_msl_update_cb)(void *args);
struct list *masterserver_list_new(void);
void masterserver_list_update(f_msl_update_cb fun, void *args);
void masterserver_list_init(struct list *masterservers);
void masterserver_list_free(void);

#endif /* !WB_MASTERSERVER_H */
