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

#ifndef WB_CMD_H
# define WB_CMD_H

# include <wb_mission.h>
# include <wb_xmpp_wf.h>
# include <wb_list.h>
# include <wb_lang.h>

/*
** Build an arg list for a whisper callback
*/

struct whisper_cb_args
{
    char *nick_to;
    char *jid_to;
};

void *cmd_whisper_args(const char *nick_to, const char *jid_to);

# define CMD_LIST_CONSOLE                                               \
    X1(add, LANG(help_add),                                             \
       XARG1(nickname))                                                 \
    X1(channel, LANG(help_channel),                                     \
       XARG1(channel))                                                  \
    X1(change, LANG(help_change),                                       \
       XARG1(mission_name))                                             \
    X1(exec, LANG(help_exec),                                           \
       XARG1(filename))                                                 \
    X0(friends, LANG(help_friends),                                     \
       XARG0())                                                         \
    X1(follow, LANG(help_follow),                                       \
       XARG1(nickname))                                                 \
    X0(help, LANG(help_help),                                           \
       XARG0())                                                         \
    X1(invite, LANG(help_invite),                                       \
       XARG1(nickname))                                                 \
    X1(last, LANG(help_last),                                           \
       XARG1(nickname))                                                 \
    X0(leave, LANG(help_leave),                                         \
       XARG0())                                                         \
    X1(master, LANG(help_master),                                       \
       XARG1(nickname))                                                 \
    X0(missions, LANG(help_missions),                                   \
       XARG0())                                                         \
    X1(name, LANG(help_name),                                           \
       XARG1(room_name))                                                \
    X1(open, LANG(help_open),                                           \
       XARG1(mission_name))                                             \
    X2(quickplay, LANG(help_quickplay),                                 \
       XARGS(cmd, XOPT(XARG1(arg1))))                                   \
    X0(quit, LANG(help_quit),                                           \
       XARG0())                                                         \
    X2(randombox, LANG(help_randombox),                                 \
       XOPT(XARGS(box, XARG1(count))))                                  \
    X1(ready, LANG(help_ready),                                         \
       XOPT(XARG1(class)))                                              \
    X1(remove, LANG(help_remove),                                       \
       XARG1(nickname))                                                 \
    X1(safe, LANG(help_safe),                                           \
       XARG1(mission_name))                                             \
    X1(say, LANG(help_say),                                             \
       XARGN(msg))                                                      \
    X1(sleep, LANG(help_sleep),                                         \
       XOPT(XARG1(n)))                                                  \
    X1(sponsor, LANG(help_sponsor),                                     \
       XARG1(sponsor))                                                  \
    X0(stats, LANG(help_stats),                                         \
       XARG0())                                                         \
    X2(stay, LANG(help_stay),                                           \
       XOPT(XARGS(n, XOPT(XARG1(unit)))))                               \
    X0(start, LANG(help_start),                                         \
       XARG0())                                                         \
    X0(switch, LANG(help_switch),                                       \
       XARG0())                                                         \
    X0(unready, LANG(help_unready),                                     \
       XARG0())                                                         \
    X2(whisper, LANG(help_whisper),                                     \
       XARGS(nickname, XARGN(msg)))                                     \
    X1(whois, LANG(help_whois),                                         \
       XARG1(nickname))                                                 \

/* Common Completions */

void complete_gameroom_players(struct list *l);
void complete_buddies(struct list *l);
void complete_friends(struct list *l);
void complete_clanmates(struct list *l);
void complete_missions_pvp(struct list *l);
void complete_missions_pve(struct list *l);
void complete_channels(struct list *l);
void complete_channels_pvp(struct list *l);
void complete_channels_pve(struct list *l);

void complete_quickplay_maps_pvp(struct list *l);

/* Whisper commands */

void cmd_follow(const char *nickname);

void cmd_invite(const char *nickname);

void cmd_leave(void);

void cmd_master(const char *nickname);

typedef void (*f_cmd_missions_cb)(const char *type, const char *setting, struct mission *m, void *args);
void cmd_missions(f_cmd_missions_cb cb, void *args);
void cmd_missions_console_cb(const char *type, const char *setting, struct mission *m, void *args);
void cmd_missions_whisper_cb(const char *type, const char *setting, struct mission *m, void *args);

void cmd_ready(const char *take_class);

void cmd_start(void);

void cmd_stay(unsigned int secs);

void cmd_switch(void);

void cmd_unready(void);

struct cmd_whois_data
{
    const char *country;
    const char *isp;

    const char *nickname;
    const char *ip;
    const char *status; /* todo: int */
    const char *profile_id;
    const char *online_id;
    unsigned int login_time;
    unsigned int rank;
};

typedef void (*f_cmd_whois_cb)(const struct cmd_whois_data *whois, void *args);

void cmd_whois(const char *nickname, f_cmd_whois_cb cb, void *args);

void cmd_whois_console_cb(const struct cmd_whois_data *whois, void *args);

void cmd_whois_whisper_cb(const struct cmd_whois_data *whois, void *args);

/* Owner-only commands */

void cmd_add(const char *nickname);

void cmd_channel(const char *channel);

void cmd_change(const char *mission_name);

void cmd_exec(const char *file_name);

void cmd_friends(void);

struct cmd_last_data
{
    const char *profile_id;
    unsigned int timestamp;
};

typedef void (*f_cmd_last_cb)(const struct cmd_last_data *last, void *args);

void cmd_last(const char *nickname, f_cmd_last_cb cb, void *args);
void cmd_last_pid(const char *profile_id, f_cmd_last_cb cb, void *args);

void cmd_last_console_cb(const struct cmd_last_data *last, void *args);

void cmd_last_whisper_cb(const struct cmd_last_data *last, void *args);

void cmd_name(const char *room_name);

void cmd_open(const char *mission_name);

void cmd_quickplay(const char *cmd, const char *arg_1);

void cmd_quit(void);

void cmd_randombox(const char *name, unsigned int count);

void cmd_remove(const char *nickname);

void cmd_safe(const char *mission_name);

void cmd_say(const char *message);

void cmd_sleep(unsigned int delay);

void cmd_sponsor(enum sponsor_type st);

typedef void (*f_cmd_stats_cb)(const char *resource, int online, void *args);
void cmd_stats_console_cb(const char *resource, int online, void *args);
void cmd_stats(f_cmd_stats_cb cb, void *args);

void cmd_whisper(const char *nickname, const char *message);

#endif /* !WB_CMD_H */
