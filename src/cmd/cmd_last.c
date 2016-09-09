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

#include <time.h>

#ifdef __MINGW32__
# define localtime_r(Time, Struct) localtime_s(Struct, Time)
#endif /* __MINGW32__ */

struct cb_args
{
    f_cmd_last_cb cb;
    void *args;
};

static void get_last_seen_date_cb(const char *profile_id,
                                  unsigned int timestamp,
                                  void *args)
{
    struct cb_args *a = (struct cb_args *) args;

    if (a->cb)
    {
        struct cmd_last_data last = {
            .profile_id = profile_id,
            .timestamp = timestamp,
        };

        a->cb(&last, a->args);
    }

    free(a);
}

static void cmd_last_cb(const char *info, void *args)
{
    struct cb_args *a = (struct cb_args *) args;

    if (info == NULL)
    {
        if (a->cb != NULL)
            a->cb(NULL, a->args);

        free(a);
        return;
    }

    char *profile_id = get_info(info, "profile_id='", "'", NULL);

    if (profile_id != NULL)
        xmpp_iq_get_last_seen_date(profile_id,
                                   get_last_seen_date_cb,
                                   a);

    free(profile_id);
}

void cmd_last_pid(const char *profile_id,
                  f_cmd_last_cb cb,
                  void *args)
{
    if (profile_id == NULL)
        return;

    struct cb_args *a = calloc(1, sizeof (struct cb_args));

    a->cb = cb;
    a->args = args;

    xmpp_iq_get_last_seen_date(profile_id,
                               get_last_seen_date_cb,
                               a);
}

void cmd_last(const char *nickname,
              f_cmd_last_cb cb,
              void *args)
{
    if (nickname == NULL)
        return;

    const char *profile_id = NULL;

    struct friend *f = friend_list_get(nickname);
    struct clanmate *c = clanmate_list_get(nickname);

    if (f != NULL)
    {
        profile_id = f->profile_id;
    }
    else if (c != NULL)
    {
        profile_id = c->profile_id;
    }

    if (profile_id == NULL)
    {
        struct cb_args *a = calloc(1, sizeof (struct cb_args));

        a->cb = cb;
        a->args = args;

        xmpp_iq_profile_info_get_status(nickname, cmd_last_cb, a);
    }
    else
    {
        cmd_last_pid(profile_id, cb, args);
    }
}

void cmd_last_console_cb(const struct cmd_last_data *last,
                         void *args)
{
    if (last == NULL)
    {
        xprintf("No such user connected\n");
    }
    else
    {
        time_t     t = last->timestamp;
        struct tm  ts;
        char       buf[64];

        localtime_r(&t, &ts);

        strftime(buf, sizeof (buf), "%a %Y-%m-%d %H:%M:%S %Z", &ts);

        xprintf("Last seen: %s\n", buf);
    }
}

void cmd_last_whisper_cb(const struct cmd_last_data *last,
                         void *args)
{
    struct whisper_cb_args *a = (struct whisper_cb_args *) args;

    if (last == NULL)
    {
       xmpp_send_message(a->nick_to, a->jid_to,
                         "Never seen this dude");
    }
    else
    {
        time_t     now = time(NULL);
        time_t     t = last->timestamp;
        struct tm  ts;
        char       buf[64];

        localtime_r(&t, &ts);

        if (t + 3600 < now)
        {
            strftime(buf, sizeof (buf), "You missed him at %Hh%M!", &ts);
        }
        else if (t + 3600 * 7 < now)
        {
            strftime(buf, sizeof (buf), "I saw him last %A", &ts);
        }
        else if (t + 3600 * 7 * 31 < now)
        {
            strftime(buf, sizeof (buf), "Not seen since %B", &ts);
        }
        else
        {
            strftime(buf, sizeof (buf), "Reported dead in %Y", &ts);
        }

        xmpp_send_message(a->nick_to, a->jid_to, buf);
    }

    free(a->nick_to);
    free(a->jid_to);
    free(a);
}

void cmd_last_wrapper(const char *nickname)
{
    cmd_last(nickname, cmd_last_console_cb, NULL);
}

int cmd_last_completions(struct list *l)
{
    complete_buddies(l);

    return 1;
}
