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

#include <wb_friend.h>
#include <wb_session.h>
#include <wb_list.h>
#include <wb_dbus.h>

#include <stdlib.h>
#include <string.h>

static int friend_cmp(const struct friend *f, const char *nickname)
{
    /* Compare nicknames, because jid is not always available */
    return strcmp(f->nickname, nickname);
}

static int friend_cmp_pid(const struct friend *f, const char *pid)
{
    /* Compare profile_id, because jid is not always available */
    return strcmp(f->profile_id, pid);
}

inline static void friend_free_fields_(struct friend *f)
{
    free(f->jid);
    free(f->nickname);
    free(f->profile_id);
}

inline static void friend_set_fields_(struct friend *f,
                                      const char *jid,
                                      const char *nickname,
                                      const char *profile_id,
                                      int status,
                                      int experience)
{
    f->jid = jid && *jid ? strdup(jid) : NULL;
    f->nickname = strdup(nickname);
    f->profile_id = strdup(profile_id);
    f->status = status;
    f->experience = experience;
}

void friend_free(struct friend *f)
{
    friend_free_fields_(f);
    free(f);
}

struct friend *friend_list_get(const char *nick)
{
    if (session.profile.friends == NULL || nick == NULL)
        return NULL;

    return list_get(session.profile.friends, nick);
}

struct friend *friend_list_get_by_pid(const char *pid)
{
    if (session.profile.friends == NULL || pid == NULL)
        return NULL;

    return list_get_by(session.profile.friends,
                       pid,
                       (f_list_cmp) friend_cmp_pid);
}

struct friend *friend_new(const char *jid,
                          const char *nickname,
                          const char *profile_id,
                          int status,
                          int experience)
{
    struct friend *f = calloc(1, sizeof (struct friend));

    friend_set_fields_(f, jid, nickname, profile_id, status, experience);

    return f;
}

struct friend *friend_list_add(const char *jid,
                               const char *nickname,
                               const char *profile_id,
                               int status,
                               int experience)
{
    struct friend *f = friend_new(jid, nickname, profile_id,
                                  status, experience);

    list_add(session.profile.friends, f);

#ifdef DBUS_API
    dbus_api_update_buddy_list();
#endif

    return f;
}

void friend_list_update(const char *jid,
                        const char *nickname,
                        const char *profile_id,
                        int status,
                        int experience)
{
    struct friend *f = list_get(session.profile.friends, nickname);

    if (!f)
        return;

    friend_free_fields_(f);

    friend_set_fields_(f, jid, nickname, profile_id, status, experience);

#ifdef DBUS_API
    dbus_api_update_buddy_list();
#endif
}

void friend_list_remove(const char *nickname)
{
    list_remove(session.profile.friends, nickname);

#ifdef DBUS_API
    dbus_api_update_buddy_list();
#endif
}

void friend_list_empty(void)
{
    list_empty(session.profile.friends);
}

void friend_list_init(void)
{
    session.profile.friends = list_new((f_list_cmp) friend_cmp,
                                       (f_list_free) friend_free);
}

void friend_list_free(void)
{
    list_free(session.profile.friends);
    session.profile.friends = NULL;
}
