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

#include <wb_tools.h>
#include <wb_geoip.h>
#include <wb_stream.h>
#include <wb_session.h>
#include <wb_xml.h>
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>
#include <wb_cmd.h>
#include <wb_log.h>
#include <wb_lang.h>

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

struct cb_args
{
    f_cmd_whois_cb cb;
    void *args;

    char *ip;
    char *nickname;
    enum status status;
    char *profile_id;
    char *online_id;
    unsigned int login_time;
    unsigned int rank;
};

static void *thread_get_geoloc(void *vargs)
{
    struct cb_args *a = (struct cb_args *) vargs;
    struct geoip *g = geoip_get_info(a->ip, 0);

    enum status i_status = a->status;
    const char *s_status =
        i_status & STATUS_AFK ? LANG(status_afk) :
        i_status & STATUS_TUTORIAL ? LANG(status_tutorial) :
        i_status & STATUS_RATING ? LANG(status_rating) :
        i_status & STATUS_PLAYING ? LANG(status_playing) :
        i_status & STATUS_SHOP ? LANG(status_shop) :
        i_status & STATUS_INVENTORY ? LANG(status_inventory) :
        i_status & STATUS_ROOM ? LANG(status_room) :
        i_status & STATUS_LOBBY ? LANG(status_lobby) :
        i_status & STATUS_ONLINE ? LANG(status_online) :
        LANG(status_offline); /* wut ? impossible !§§!§ */

    if (a->cb)
    {
        struct cmd_whois_data whois = {
            .country = g->country_name,
            .isp = g->isp,

            .nickname = a->nickname,
            .ip = a->ip,
            .status = s_status, /* todo: int */
            .profile_id = a->profile_id,
            .online_id = a->online_id,
            .login_time = a->login_time,
            .rank = a->rank
        };

        a->cb(&whois, a->args);
    }

    geoip_free(g);

    free(a->nickname);
    free(a->ip);
    free(a->online_id);
    free(a->profile_id);
    free(a);

    pthread_exit(NULL);
}

static void cmd_whois_cb(const char *info, void *args)
{
    struct cb_args *a = (struct cb_args *) args;

    if (info == NULL)
    {
        if (a->cb)
            a->cb(NULL, a->args);

        free(a);
    }
    else
    {
        a->status = get_info_int(info, "status='", "'", NULL);
        a->ip = get_info(info, "ip_address='", "'", NULL);
        a->nickname = get_info(info, "nickname='", "'", NULL);
        a->profile_id = get_info(info, "profile_id='", "'", NULL);
        a->online_id = get_info(info, "online_id='", "'", NULL);
        a->login_time = get_info_int(info, "login_time='", "'", NULL);
        a->rank = get_info_int(info, "rank='", "'", NULL);

        pthread_t thread_gl;

        if (pthread_create(&thread_gl, NULL, thread_get_geoloc, args) == -1)
            perror("pthread_create");

        pthread_detach(thread_gl);
    }
}

void *cmd_whisper_args(const char *nick_to, const char *jid_to)
{
    struct whisper_cb_args *a = calloc(1, sizeof (struct whisper_cb_args));

    if (nick_to != NULL)
        a->nick_to = strdup(nick_to);
    if (jid_to != NULL)
        a->jid_to = strdup(jid_to);

    return (void *) a;
}

void cmd_whois(const char *nickname,
               f_cmd_whois_cb cb,
               void *args)
{
    struct cb_args *a = calloc(1, sizeof (struct cb_args));

    a->cb = cb;
    a->args = args;

    xmpp_iq_profile_info_get_status(nickname, cmd_whois_cb, a);
}

void cmd_whois_console_cb(const struct cmd_whois_data *whois,
                          void *args)
{
    if (whois == NULL)
        xprintf("%s", LANG(error_no_user));
    else if (whois->country == NULL)
        xprintf("%s (ip: %s) - %s - %s %d",
                whois->nickname,
                whois->ip,
                whois->status,
                LANG(rank),
                whois->rank);
    else
        xprintf("%s (ip: %s - %s - %s) - %s - %s %d",
                whois->nickname,
                whois->ip,
                whois->country,
                whois->isp,
                whois->status,
                LANG(rank),
                whois->rank);
}

void cmd_whois_whisper_cb(const struct cmd_whois_data *whois,
                          void *args)
{
    struct whisper_cb_args *a = (struct whisper_cb_args *) args;

    if (whois == NULL)
    {
       xmpp_send_message(a->nick_to, a->jid_to,
                         LANG(whisper_whois_unknown));
    }
    else
    {
        char *message;

        if (whois->country == NULL)
        {
            message = LANG_FMT(whisper_whois_no_country,
                               whois->status);
        }
        else
        {
            const char *c = whois->country;
            const char *s = whois->status;
            int r = time(NULL) % 3;

            message =
                (r == 0) ? LANG_FMT(whisper_whois_1, c, s):
                (r == 1) ? LANG_FMT(whisper_whois_2, c, s):
                           LANG_FMT(whisper_whois_3, c, s);
        }

        xmpp_send_message(a->nick_to, a->jid_to, message);
        free(message);
    }

    free(a->nick_to);
    free(a->jid_to);
    free(a);
}

void cmd_whois_wrapper(const char *nickname)
{
    cmd_whois(nickname, cmd_whois_console_cb, NULL);
}

int cmd_whois_completions(struct list *l)
{
    complete_buddies(l);

    return 1;
}
