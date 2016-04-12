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

#ifndef WB_XMPP
#define WB_XMPP

#include <string.h>

#define XMPP_ID "uid%08d"

enum xmpp_msg_type
{
	XMPP_TYPE_OTHER = 1 << 0,
	XMPP_TYPE_ERROR = 1 << 1,
	XMPP_TYPE_GET = 1 << 2,
	XMPP_TYPE_RESULT = 1 << 3,
	XMPP_TYPE_NONE = 1 << 4
};

/* Query ID handler */

typedef struct { char uid[ 12 ]; } t_uid;
typedef void ( *f_id_callback )( const char *msg,
enum xmpp_msg_type type,
	void *args );

void idh_register ( const t_uid *id, int permanent,
					f_id_callback callback, void *args );

int idh_handle ( const char *msg_id,
				 const char *msg,
enum xmpp_msg_type type );

void idh_generate_unique_id ( t_uid *id );

/* Query Stanza handler */

typedef void ( *f_query_callback )( const char *msg_id,
									const char *msg,
									void *args );

void qh_register ( const char *query, int permanent,
				   f_query_callback callback, void *args );

int qh_handle ( const char *query,
				const char *msg_id,
				const char *msg );

/* XMPP Tools */

char *get_msg_id ( const char *msg );

enum xmpp_msg_type get_msg_type ( const char *msg );

char *get_query_tag_name ( const char *msg );

char *sasl_combine_logins ( const char *login, const char *pwd );

/* Sent Queries */

typedef void ( *f_stream_cb )( void *args );
void xmpp_stream ( const char *login, const char *password,
				   f_stream_cb cb, void *args );

void xmpp_starttls ( const char *login, const char *password,
					 f_stream_cb cb, void *args );

typedef void ( *f_sasl_cb )( void *args );
void xmpp_sasl ( const char *login, const char *password,
				 f_sasl_cb cb, void *args );

typedef void ( *f_bind_cb )( void *args );
void xmpp_bind ( const char *resource, f_bind_cb cb, void *args );

typedef void ( *f_session_cb )( void *args );
void xmpp_iq_session ( f_session_cb cb, void *args );


typedef void ( *f_presence_cb )( const char *room, int leave, void *args );
void xmpp_presence ( const char *room_jid, int leave,
					 f_presence_cb cb, void *args );
void xmpp_room_message ( const char *room_jid, const char *msg );
void xmpp_iq_ping ( void );

/* Received Queries */

#define XMPP_REGISTER_QUERY_HDLR()                 \
    do {                                            \
        void xmpp_iq_ping_r(void);                  \
                                                    \
        xmpp_iq_ping_r();                           \
    } while (0);

#endif /* !WB_XMPP */
