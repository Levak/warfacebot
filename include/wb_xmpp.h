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

#ifndef WB_XMPP
# define WB_XMPP

# define XMPP_ID "uid%08d"

typedef struct { char uid[12]; } t_uid;
typedef void (*f_id_callback)(const char *msg, void *args);

void idh_register(const t_uid *id, int permanent,
                  f_id_callback callback, void *args);
int idh_handle(const char *msg_id, const char *msg);
void idh_generate_unique_id(t_uid *id);


typedef void (*f_query_callback)(const char *msg_id, const char *msg, void *args);

void qh_register(const char *query, int permanent,
                 f_query_callback callback, void *args);
int qh_handle(const char *query, const char *msg_id, const char *msg);


char *get_msg_id(const char *msg);
char *get_query_tag_name(const char *msg);
int xmpp_is_error(const char *msg);

char *sasl_combine_logins(const char *login, const char *pwd);

void xmpp_connect(int fd, const char *login, const char *pass);
void xmpp_close(int fd);

/* Sent Queries */

void xmpp_presence(const char *room_jid, int leave);
void xmpp_room_message(const char *room_jid, const char *msg);

/* Received Queries */

# define XMPP_REGISTER_QUERY_HDLR()                 \
    do {                                            \
        void xmpp_iq_ping_r(void);                  \
                                                    \
        xmpp_iq_ping_r();                           \
    } while (0);

#endif /* !WB_XMPP */
