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

#include <wb_list.h>

#include <stdint.h>
#include <stdlib.h>

struct node
{
    void *value;
    struct node *next;
};

void list_add(struct list *l, void *value)
{
    struct node *n = calloc(sizeof (struct node), 1);
    struct node *h = l->head;
    struct node *p = NULL;

    while (h != NULL)
    {
        p = h;
        h = h->next;
    }

    if (p == NULL)
        l->head = n;
    else
        p->next = n;

    n->value = value;

    l->length++;
}

void list_empty(struct list *l)
{
    struct node *h = l->head;

    while (h != NULL)
    {
        struct node *n = h->next;
        free(h);
        h = n;
    }
}

struct list *list_new(f_list_cmp cmp)
{
    struct list *l = calloc(sizeof (struct list), 1);

    l->cmp = cmp;

    return l;
}

void list_free(struct list *l)
{
    list_empty(l);
    free(l);
}

void list_foreach(struct list *l, f_list_callback func)
{
    struct node *h = l->head;

    while (h != NULL)
    {
        struct node *n = h->next;
        func(h->value);
        h = n;
    }
}

int list_contains(struct list *l, void *value)
{
    struct node *h = l->head;

    while (h != NULL)
    {
        struct node *n = h->next;
        if (l->cmp(h->value, value) == 0)
            return 1;
        h = n;
    }

    return 0;
}
