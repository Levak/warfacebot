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

#ifndef WB_CMD_H
# define WB_CMD_H

# include <wb_mission.h>
# include <wb_xmpp_wf.h>
# include <wb_list.h>

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
    X1(add, "Send a friend request",                                    \
       XARG1(nickname))                                                 \
    X1(channel, "Change channel",                                       \
       XARG1(channel))                                                  \
    X1(change, "Change mission/map",                                    \
       XARG1(mission_name))                                             \
    X1(exec, "Execute commands from a file",                            \
       XARG1(filename))                                                 \
    X0(friends, "Display friends and clanmates",                        \
       XARG0())                                                         \
    X1(follow, "Try to follow someone",                                 \
       XARG1(nickname))                                                 \
    X0(help, "Display the command line help",                           \
       XARG0())                                                         \
    X1(invite, "Invite someone to a room",                              \
       XARG1(nickname))                                                 \
    X1(last, "Display the last seen date of someone",                   \
       XARG1(nickname))                                                 \
    X0(leave, "Leave the room",                                         \
       XARG0())                                                         \
    X1(master, "Give back room host",                                   \
       XARG1(nickname))                                                 \
    X0(missions, "Display the crown challenge objectives",              \
       XARG0())                                                         \
    X1(name, "Rename the room",                                         \
       XARG1(room_name))                                                \
    X1(open, "Open a new room",                                         \
       XARG1(mission_name))                                             \
    X2(quickplay, "Quickplay match (open/cancel/invite/start)",         \
       XARGS(cmd, XOPT(XARG1(arg1))))                                   \
    X0(quit, "Quit warfacebot",                                         \
       XARG0())                                                         \
    X2(randombox, "Open/list a randombox(es)",                          \
       XOPT(XARGS(box, XARG1(count))))                                  \
    X1(ready, "Become ready and/or change class",                       \
       XOPT(XARG1(class)))                                              \
    X1(remove, "Remove someone from the friend list",                   \
       XARG1(nickname))                                                 \
    X1(safe, "Open a new safe room",                                    \
       XARG1(mission_name))                                             \
    X1(say, "Say something on the room chat",                           \
       XARGN(msg))                                                      \
    X1(sleep, "Idle the readline for n seconds",                        \
       XOPT(XARG1(n)))                                                  \
    X1(sponsor, "Change sponsor (weapon/outfit/equipment)",             \
       XARG1(sponsor))                                                  \
    X0(stats, "Display channel load stats",                             \
       XARG0())                                                         \
    X2(stay, "Stay in the room for n x unit (default: seconds)",        \
       XOPT(XARGS(n, XOPT(XARG1(unit)))))                               \
    X0(start, "Start the room",                                         \
       XARG0())                                                         \
    X0(switch, "Switch team side",                                      \
       XARG0())                                                         \
    X0(unready, "Become unready",                                       \
       XARG0())                                                         \
    X2(whisper, "Send a private message",                               \
       XARGS(nickname, XARGN(msg)))                                     \
    X1(whois, "Display someone status and country",                     \
       XARG1(nickname))                                                 \

/* Common Completions */

void complete_gameroom_players(struct list *l);
void complete_buddies(struct list *l);
void complete_friends(struct list *l);
void complete_clanmates(struct list *l);
void complete_missions_pvp(struct list *l);
void complete_missions_pve(struct list *l);

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
    char *country;
    char *isp;

    char *nickname;
    char *ip;
    const char *status; /* todo: int */
    char *profile_id;
    char *online_id;
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
