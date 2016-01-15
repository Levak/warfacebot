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

#include <gio/gio.h>

#include <wb_dbus_methods.h>

#include <wb_session.h>
#include <wb_list.h>
#include <wb_friend.h>
#include <wb_clanmate.h>

struct array
{
    guint i;
    gchar **tab;
};

static void flist_to_array(struct friend *f, struct array *arr)
{
    arr->tab[arr->i++] = f->nickname;
}

static void clist_to_array(struct clanmate *c, struct array *arr)
{
    arr->tab[arr->i++] = c->nickname;
}

/*
** DBus method call: "Buddies"
*/
gboolean on_handle_buddies(Warfacebot *object,
                           GDBusMethodInvocation *invocation)
{
    struct list *fl = session.friends;
    struct list *cl = session.clanmates;

    struct array *farr = malloc(sizeof (struct array));
    struct array *carr = malloc(sizeof (struct array));

    farr->tab = malloc((1 + fl->length) * sizeof (gchar *));
    carr->tab = malloc((1 + cl->length) * sizeof (gchar *));

    farr->i = 0;
    carr->i = 0;

    list_foreach(fl, (f_list_callback) flist_to_array, farr);
    list_foreach(cl, (f_list_callback) clist_to_array, carr);

    farr->tab[farr->i] = NULL;
    carr->tab[carr->i] = NULL;

    warfacebot_complete_buddies(
        object,
        invocation,
        (const gchar * const *) farr->tab,
        (const gchar * const *) carr->tab);

    free(farr->tab);
    free(carr->tab);
    free(farr);
    free(carr);

    return TRUE;
}
