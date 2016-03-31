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

/*
** Build an arg list for a whisper callback
*/

struct whisper_cb_args
{
    char *nick_to;
    char *jid_to;
};

void *cmd_whisper_args(const char *nick_to, const char *jid_to);


/* Whisper commands */

void cmd_follow(const char *nickname);

void cmd_invite(const char *nickname, int force);

void cmd_leave(void);

void cmd_master(const char *nickname);

typedef void (*f_cmd_missions_cb)(const char *type, const char *setting, struct mission *m, void *args);
void cmd_missions(f_cmd_missions_cb cb, void *args);
void cmd_missions_console_cb(const char *type, const char *setting, struct mission *m, void *args);
void cmd_missions_whisper_cb(const char *type, const char *setting, struct mission *m, void *args);

void cmd_ready(const char *take_class);

void cmd_start(void);

void cmd_switch(void);

typedef void (*f_cmd_whois_cb)(const char *ip, const char *country, const char *status, void *args);
void cmd_whois(const char *nickname, f_cmd_whois_cb cb, void *args);
void cmd_whois_console_cb(const char *ip, const char *country, const char *status, void *args);
void cmd_whois_whisper_cb(const char *ip, const char *country, const char *status, void *args);


/* Owner-only commands */

void cmd_channel(const char *channel);

void cmd_change(const char *mission_name);

void cmd_friends(void);

void cmd_name(const char *room_name);

void cmd_add_friend(const char *nickname);

void cmd_remove_friend(const char *nickname);

void cmd_say(const char *message);

typedef void (*f_cmd_stats_cb)(const char *resource, int online, void *args);
void cmd_stats_console_cb(const char *resource, int online, void *args);
void cmd_stats(f_cmd_stats_cb cb, void *args);

void cmd_safe(const char *mission_name);

void cmd_open(const char *mission_name);

void cmd_whisper(const char *nickname, const char *message);


#endif /* !WB_CMD_H */
