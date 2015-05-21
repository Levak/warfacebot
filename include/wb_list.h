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

#ifndef WB_LIST_H
# define WB_LIST_H

#include <stdlib.h>

typedef void (*f_list_callback)(void *value);
typedef int (*f_list_cmp)(const void *a, const void *b);
typedef void (*f_list_free)(void *e);

struct list
{
    struct node *head;
    f_list_cmp cmp;
    f_list_free free;
    size_t length;
};

void list_add(struct list *l, void *value);
void list_empty(struct list *l);
struct list *list_new(f_list_cmp cmp_func, f_list_free free_func);
void list_free(struct list *l);
void list_foreach(struct list *l, f_list_callback func);
void *list_get(struct list *l, const void *value);
int list_contains(struct list *l, const void *value);

#endif /* !WB_LIST_H */
