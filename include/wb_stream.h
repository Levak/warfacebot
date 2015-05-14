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

#ifndef WB_STREAM_H
# define WB_STREAM_H

#include <stdint.h>

void send_stream(int fd, char *msg, uint32_t msg_size);
void send_stream_ascii(int fd, char *msg);
void flush_stream(int fd);

void send_stream_format(int fd, char *fmt, ...);

char *read_stream_keep(int fd);
int read_stream(int fd);

int connect_wf(char *hostname, int port);

#endif /* !WB_STREAM_H */
