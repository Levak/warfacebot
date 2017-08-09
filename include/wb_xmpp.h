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

#ifndef WB_XMPP
# define WB_XMPP

# include <string.h>

# include <wb_cvar.h>

# define XMPP_ID "uid%08d"

enum xmpp_chunk_state
{
    XMPP_CHUNK_CACHED = 1,
    XMPP_CHUNK_MORE = 2,
    XMPP_CHUNK_END = 3,
};

enum xmpp_msg_type
{
    XMPP_TYPE_ERROR   = 1 << 0,
    XMPP_TYPE_SET     = 1 << 1,
    XMPP_TYPE_GET     = 1 << 2,
    XMPP_TYPE_RESULT  = 1 << 3
};

/* Query ID handler */

typedef struct { char uid[12]; } t_uid;
typedef void (*f_id_callback)(const char *msg,
                              enum xmpp_msg_type type,
                              void *args);

void idh_init(void);

void idh_register(const t_uid *id, int permanent,
                  f_id_callback callback, void *args);

int idh_handle(const char *msg_id,
               const char *msg,
               enum xmpp_msg_type type);

void idh_generate_unique_id(t_uid *id);

/* Query Stanza handler */

typedef void (*f_query_callback)(const char *msg_id,
                                 const char *msg,
                                 void *args);

void qh_register(const char *query, int permanent,
                 f_query_callback callback, void *args);

void qh_remove(const char *query);

int qh_handle(const char *query,
              const char *msg_id,
              const char *msg);

/* XMPP Functions */

void xmpp_send(const char *fmt,
               ...);

struct jid
{
    char *node;
    char *domain;
    char *resource;
};

# define JID(Str) &((const struct jid) {     \
            .node = NULL,                    \
            .domain = (char *) (Str),        \
            .resource = NULL                 \
        })

# define JID_HOST JID(session.online.jid.host)

void xmpp_send_iq(const struct jid *target,
                  enum xmpp_msg_type type,
                  f_id_callback cb,
                  void *args,
                  const char *fmt,
                  ...);

# define xmpp_send_iq_result(Target, Id, Fmt, ...)      \
    xmpp_send_iq(Target, XMPP_TYPE_RESULT,              \
                 NULL, (void *) Id,                     \
                 Fmt, __VA_ARGS__)

# define xmpp_send_iq_get(Target, Cb, Args, Fmt, ...)   \
    xmpp_send_iq(Target, XMPP_TYPE_GET,                 \
                 Cb, Args,                              \
                 Fmt, __VA_ARGS__)

# define xmpp_send_iq_set(Target, Cb, Args, Fmt, ...)   \
    xmpp_send_iq(Target, XMPP_TYPE_SET,                 \
                 Cb, Args,                              \
                 Fmt, __VA_ARGS__)

/* XMPP Tools */

char *get_msg_id(const char *msg);

enum xmpp_msg_type get_msg_type(const char *msg);

char *get_query_tag_name(const char *msg);

char *sasl_combine_logins(const char *login, const char *pwd);

/* Sent Queries */

typedef void (*f_stream_cb)(void *args);
void xmpp_stream(const char *login, const char *password,
                 f_stream_cb cb, void *args);

void xmpp_starttls(const char *login, const char *password,
                 f_stream_cb cb, void *args);

typedef void (*f_sasl_cb)(void *args);
void xmpp_sasl(const char *login, const char *password,
               f_sasl_cb cb, void *args);

typedef void (*f_bind_cb)(void *args);
void xmpp_bind(const char *resource, f_bind_cb cb, void *args);

typedef void (*f_session_cb)(void *args);
void xmpp_iq_session(f_session_cb cb, void *args);

enum xmpp_presence_type
{
    XMPP_PRESENCE_JOIN,
    XMPP_PRESENCE_LEAVE,
};

typedef void (*f_presence_cb)(const char *room,
                              enum xmpp_presence_type type,
                              void *args);
void xmpp_presence(const char *room_jid,
                   enum xmpp_presence_type type,
                   f_presence_cb cb, void *args);

void xmpp_room_message(const char *room_jid, const char *msg);

void xmpp_iq_ping(void);

/* Received Queries */

# define XMPP_REGISTER_QUERY_HDLR()                 \
    do {                                            \
        void xmpp_iq_ping_r(void);                  \
        void xmpp_presence_r(void);                 \
                                                    \
        xmpp_iq_ping_r();                           \
        xmpp_presence_r();                          \
    } while (0);

#endif /* !WB_XMPP */
