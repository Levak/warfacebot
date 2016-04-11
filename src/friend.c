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

#include <wb_friend.h>
#include <wb_session.h>
#include <wb_list.h>
#include <wb_dbus.h>
#include <wb_tools.h>

#include <stdlib.h>
#include <string.h>

static int friend_cmp(const struct friend *f, const char *nickname)
{
    /* Compare nicknames, because jid is not always available */
    return strcmp(f->nickname, nickname);
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
									  int experience,
									  char *place_token,
									  char *place_info_token,
									  char *mode_info_token,
									  char *mission_info_token)
{
    f->jid = jid && *jid ? strdup(jid) : NULL;
    f->nickname = strdup(nickname);
    f->profile_id = strdup(profile_id);
    f->status = status;
    f->experience = experience;
	f->place_token = strdup(place_token ? place_token : "");
	f->place_info_token = strdup(place_info_token ? place_info_token : "");
	f->mode_info_token = strdup(mode_info_token ? mode_info_token : "");
	f->mission_info_token = strdup(mission_info_token ? mission_info_token : "");
}

static void friend_free(struct friend *f)
{
    friend_free_fields_(f);
    free(f);
}

void friend_list_add(const char *jid,
                     const char *nickname,
                     const char *profile_id,
                     int status,
                     int experience)
{
    struct friend *f = calloc(1, sizeof (struct friend));

    friend_set_fields_(f, jid, nickname, profile_id, status, experience, "", "", "", "");

    list_add(session.friends, f);

#ifdef DBUS_API
    dbus_api_update_buddy_list();
#endif
}

void friend_list_update(const char *jid,
                        const char *nickname,
                        const char *profile_id,
                        int status,
						int experience,
						char *place_token,
						char *place_info_token,
						char *mode_info_token,
						char *mission_info_token)
{
    struct friend *f = list_get(session.friends, nickname);

    if (!f)
        return;

	if ((!(f->status & STATUS_ONLINE) || (f->status & (STATUS_AFK | STATUS_PLAYING))) &&
		 (!(status & (STATUS_AFK | STATUS_PLAYING)) && (status != STATUS_OFFLINE)))
		LOGPRINT("%-20s " KGRN BOLD "%s\n", "PLAYER ONLINE", nickname);
	if (!(f->status & STATUS_AFK) && (status & (STATUS_AFK & ~STATUS_PLAYING)))
		LOGPRINT("%-20s " KYEL BOLD "%s\n", "PLAYER AFK", nickname);
	if (!(f->status & STATUS_PLAYING) && (status & (STATUS_PLAYING & ~STATUS_AFK)))
	{
		char *map;
		if (strstr(place_token, "pve"))
			FORMAT(map, "%s", place_info_token);
		else if (strstr(place_token, "pvp"))
			FORMAT(map, "@%s", mission_info_token + sizeof("@pvp_mission_display_name"));
		else
			map = strdup("unknown");
		LOGPRINT("%-20s " KMAG BOLD "%-20s " KRST BOLD "%s\n", "PLAYER INGAME", nickname, map);
		free(map);
	}
	if ((f->status & STATUS_ONLINE) && status == STATUS_OFFLINE)
		LOGPRINT("%-20s " KCYN BOLD "%s\n", "PLAYER OFFLINE", nickname);

    friend_free_fields_(f);

    friend_set_fields_(f, jid, nickname, profile_id, status, experience,
					   place_token, place_info_token, mode_info_token, mission_info_token);

#ifdef DBUS_API
    dbus_api_update_buddy_list();
#endif
}

void friend_list_remove(const char *nickname)
{
    list_remove(session.friends, nickname);
}

void friend_list_empty(void)
{
    list_empty(session.friends);
}

void friend_list_init(void)
{
    session.friends = list_new((f_list_cmp) friend_cmp,
                               (f_list_free) friend_free);
}

void friend_list_free(void)
{
    list_free(session.friends);
    session.friends = NULL;
}
