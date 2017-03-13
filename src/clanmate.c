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

#include <wb_clanmate.h>
#include <wb_session.h>
#include <wb_list.h>
#include <wb_dbus.h>

#include <stdlib.h>
#include <string.h>

static int clanmate_cmp(const struct clanmate *c, const char *nick)
{
    return strcmp(c->nickname, nick);
}

static int clanmate_cmp_pid(const struct clanmate *c, const char *pid)
{
    /* Compare profile_id, because jid/nick are not always available */
    return strcmp(c->profile_id, pid);
}

static inline void clanmate_free_fields_(struct clanmate *f)
{
    free(f->jid);
    free(f->nickname);
    free(f->profile_id);
}

static inline void clanmate_set_fields_(struct clanmate *f,
                                        const char *jid,
                                        const char *nickname,
                                        const char *profile_id,
                                        int status,
                                        int experience,
                                        int clan_points,
                                        int clan_role)
{
    f->jid = jid && *jid ? strdup(jid) : NULL;
    f->nickname = strdup(nickname);
    f->profile_id = strdup(profile_id);
    f->status = status;
    f->experience = experience;
    f->clan_points = clan_points;
    f->clan_role = clan_role;
}

void clanmate_free(struct clanmate *f)
{
    clanmate_free_fields_(f);
    free(f);
}

struct clanmate *clanmate_list_get(const char *nick)
{
    if (session.profile.clanmates == NULL || nick == NULL)
        return NULL;

    return list_get(session.profile.clanmates, nick);
}

struct clanmate *clanmate_list_get_by_pid(const char *pid)
{
    if (session.profile.clanmates == NULL || pid == NULL)
        return NULL;

    return list_get_by(session.profile.clanmates,
                       pid,
                       (f_list_cmp) clanmate_cmp_pid);
}

struct clanmate *clanmate_new(const char *jid,
                              const char *nickname,
                              const char *profile_id,
                              int status,
                              int experience,
                              int clan_points,
                              int clan_role)
{
    struct clanmate *c = calloc(1, sizeof (struct clanmate));

    clanmate_set_fields_(c, jid, nickname, profile_id, status, experience,
                         clan_points, clan_role);

    return c;
}

static void _compute_own_position(const struct clanmate *c, void *args)
{
    if (session.profile.clan.points > c->clan_points)
    {
        unsigned int *p_own_position = (unsigned int *) args;

        --*p_own_position;
    }
}

static struct clanmate *clanmate_list_add(const char *jid,
                                          const char *nickname,
                                          const char *profile_id,
                                          int status,
                                          int experience,
                                          int clan_points,
                                          int clan_role)
{
    struct clanmate *c = clanmate_new(jid, nickname, profile_id,
                                      status, experience,
                                      clan_points, clan_role);

    list_add(session.profile.clanmates, c);

#ifdef DBUS_API
    dbus_api_update_buddy_list();
#endif

    return c;
}

static void clanmate_list_remove(const char *profile_id)
{
    list_remove(session.profile.clanmates, profile_id);
}

enum clan_update clanmate_list_update(const char *jid,
                                      const char *nickname,
                                      const char *profile_id,
                                      int status,
                                      int experience,
                                      int clan_points,
                                      int clan_role)
{
    enum clan_update ret;
    struct clanmate *f = clanmate_list_get_by_pid(profile_id);

    if (!f)
    {
        clanmate_list_add(jid, nickname, profile_id, status, experience,
                          clan_points, clan_role);
        ret = CLAN_UPDATE_JOINED;
    }
    else if (nickname == NULL)
    {
        clanmate_list_remove(profile_id);
        ret = CLAN_UPDATE_LEFT;
    }
    else
    {
        clanmate_free_fields_(f);

        clanmate_set_fields_(f, jid, nickname, profile_id, status, experience,
                             clan_points, clan_role);
        ret = CLAN_UPDATE_CHANGED;
    }

    {
        unsigned int own_position =
            session.profile.clanmates->length + 1;

        list_foreach(session.profile.clanmates,
                     (f_list_callback) _compute_own_position,
                     &own_position);

        session.profile.clan.own_position = own_position;
    }

#ifdef DBUS_API
    dbus_api_update_buddy_list();
#endif

    return ret;
}

void clanmate_list_empty(void)
{
    list_empty(session.profile.clanmates);
}

void clanmate_list_init(void)
{
    session.profile.clanmates = list_new((f_list_cmp) clanmate_cmp,
                                         (f_list_free) clanmate_free);
}

void clanmate_list_free(void)
{
    list_free(session.profile.clanmates);
    session.profile.clanmates = NULL;
}
