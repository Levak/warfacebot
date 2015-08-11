/**
 * WarfaceBot, a blind XMPP client for Warface (FPS)
 * Copyright (C) 2015 Levak Borok <levak92@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef WB_CMD_H
# define WB_CMD_H

void cmd_invite(const char *nickname, int force);

void cmd_leave(void);

void cmd_master(const char *nickname);

void cmd_missions(const char *nick_to, const char *jid_to);

void cmd_ready(const char *take_class);

void cmd_remove_friend(const char *nickname);

void cmd_say(const char *message);

void cmd_whois(const char *nickname, const char *nick_to, const char *jid_to);

#endif /* !WB_CMD_H */
