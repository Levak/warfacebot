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

# include <stdint.h>

int connect_wf(const char *hostname, int port);

void send_stream(int fd, char *msg, uint32_t msg_size);
void send_stream_ascii(int fd, char *msg);
void flush_stream(int fd);

void send_stream_format(int fd, const char *fmt, ...);
void send_stream_ack(int fd);
char *read_stream(int fd);

void crypt_init(int key);
int crypt_is_ready(void);
void crypt_decrypt(uint8_t *buff, int len);
void crypt_encrypt(uint8_t *buff, int len);

# ifdef USE_TLS
#  include <sys/types.h>

int init_tls_stream(int fd);
void close_tls_stream(void);
void free_tls_stream(void);
ssize_t tls_recv(int fd, void *buf, size_t count);
ssize_t tls_send(int fd, const void *buf, size_t count);

# endif /* USE_TLS */

#endif /* !WB_STREAM_H */
