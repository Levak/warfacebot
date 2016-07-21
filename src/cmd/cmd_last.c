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

#include <wb_session.h>
#include <wb_tools.h>
#include <wb_xmpp_wf.h>
#include <wb_log.h>
#include <wb_cmd.h>

static void get_last_seen_date_cb(const char *profile_id,
                                  unsigned int timestamp,
                                  void *args)
{
    time_t     now = timestamp;
    struct tm  ts;
    char       buf[80];

    ts = *localtime(&now);

    strftime(buf, sizeof (buf), "%a %Y-%m-%d %H:%M:%S %Z", &ts);

    xprintf("Last seen: %s\n", buf);
}

static void cmd_last_cb(const char *info, void *args)
{
    if (info == NULL)
    {
        xprintf("No such user connected\n");
        return;
    }

    char *profile_id = get_info(info, "profile_id='", "'", NULL);

    if (profile_id != NULL)
        xmpp_iq_get_last_seen_date(profile_id,
                                   get_last_seen_date_cb,
                                   NULL);

    free(profile_id);
}

void cmd_last(const char *nickname)
{
    if (nickname == NULL)
        return;

    const char *profile_id = NULL;

    struct friend *f = list_get(session.profile.friends, nickname);
    struct clanmate *c = list_get(session.profile.clanmates, nickname);

    if (f != NULL)
    {
        profile_id = f->profile_id;
    }
    else if (c != NULL)
    {
        profile_id = c->profile_id;
    }

    if (profile_id != NULL)
    {
        xmpp_iq_get_last_seen_date(profile_id,
                                   get_last_seen_date_cb,
                                   NULL);
    }
    else
    {
        xmpp_iq_profile_info_get_status(nickname, cmd_last_cb, NULL);
    }
}

void cmd_last_wrapper(const char *nickname)
{
    cmd_last(nickname);
}

int cmd_last_completions(struct list *l)
{
    complete_buddies(l);

    return 1;
}
