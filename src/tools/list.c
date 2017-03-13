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

#include <wb_list.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct node
{
    void *value;
    struct node *next;
};

void list_add(struct list *l, void *value)
{
    struct node *n = calloc(sizeof (struct node), 1);
    struct node *h = l->head;

    n->next = h;
    n->value = value;

    l->head = n;
    l->length++;

    if (l->tail == NULL)
        l->tail = n;
}


void list_remove(struct list *l, const void *value)
{
    struct node *h = l->head;
    struct node *p = NULL;

    while (h != NULL)
    {
        if (l->cmp(h->value, value) == 0)
        {
            if (p == NULL)
                l->head = h->next;
            else
                p->next = h->next;

            if (p == NULL)
                l->tail = NULL;
            else if (h->next == NULL)
                l->tail = p;

            l->free(h->value);
            free(h);
            l->length--;

            break;
        }

        p = h;
        h = h->next;
    };
}

static void nullfree_(void *e)
{
    return;
}

void list_empty(struct list *l)
{
    struct node *h = l->head;
    f_list_free f = l->free;

    if (f == NULL)
        f = (f_list_free) nullfree_;

    while (h != NULL)
    {
        struct node *n = h->next;

        f(h->value);
        free(h);
        h = n;
    }

    l->length = 0;
    l->head = NULL;
    l->tail = NULL;
}

struct list *list_new(f_list_cmp cmp_func, f_list_free free_func)
{
    struct list *l = calloc(sizeof (struct list), 1);

    l->cmp = cmp_func;
    l->free = free_func;

    return l;
}

void list_free(struct list *l)
{
    list_empty(l);
    free(l);
}

void list_foreach(struct list *l, f_list_callback func, void *args)
{
    struct node *h = l->head;

    while (h != NULL)
    {
        struct node *n = h->next;
        func(h->value, args);
        h = n;
    }
}

void *list_get_by(struct list *l, const void *value, f_list_cmp cmp)
{
    struct node *h = l->head;

    while (h != NULL)
    {
        struct node *n = h->next;
        if (cmp(h->value, value) == 0)
            return h->value;
        h = n;
    }

    return NULL;
}

void *list_get(struct list *l, const void *value)
{
    return list_get_by(l, value, l->cmp);
}

void *list_first(struct list *l)
{
    struct node *h = l->head;

    return (h != NULL) ? h->value : NULL;
}

void *list_last(struct list *l)
{
    struct node *t = l->tail;

    return (t != NULL) ? t->value : NULL;
}

int list_contains(struct list *l, const void *value)
{
    return list_get(l, value) != NULL;
}

/* Readline completion generator from a list */

static struct list *active_generator_list = NULL;

void list_rl_set(struct list *l)
{
    active_generator_list = l;
}

void list_rl_init(struct list *l,
                  f_list_rl_match match,
                  f_list_rl_copy copy)
{
    l->rl_match = match;
    l->rl_copy = copy;
}

char *list_rl_generator(const char *text, int state)
{
    struct list *l = active_generator_list;

    /* New completion request */
    if (!state)
    {
        if (l == NULL)
            return NULL;

        l->rl_curr = l->head;
        l->rl_len = strlen(text);
    }

    /* Go through all items in the list starting from
       previous completion */
    while (l->rl_curr != NULL)
    {
        struct node *curr = l->rl_curr;
        l->rl_curr = l->rl_curr->next;

        /* If we matched something, return a copy of it */
        if (0 == l->rl_match(curr->value, text, l->rl_len))
        {
            return l->rl_copy(curr->value);
        }
    }

    return NULL;
}
