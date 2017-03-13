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

#ifndef WB_STREAM_H
# define WB_STREAM_H

# include <stdint.h>

int stream_connect(const char *hostname, int port);
void stream_send_msg(int fd, const char *msg);
void stream_send_ack(int fd);
void stream_flush(int fd);
char *stream_read(int fd);

void crypt_init(int salt);
int crypt_is_ready(void);
void crypt_decrypt(uint8_t *buff, int len);
void crypt_encrypt(uint8_t *buff, int len);

# ifdef USE_TLS
#  include <sys/types.h>

int tls_init(int fd);
void tls_close(void);
void tls_free(void);
ssize_t tls_recv(int fd, void *buf, size_t count);
ssize_t tls_send(int fd, const void *buf, size_t count);
void tls_perror(const char *s, int ret);

# endif /* USE_TLS */

#endif /* !WB_STREAM_H */
