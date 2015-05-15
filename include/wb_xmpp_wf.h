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

#ifndef WB_XMPP_WF_H
# define WB_XMPP_WF_H

char *wf_get_query_content(const char *msg);


void xmpp_send_message(int wfs,
                       const char *from_login, const char *from_jid,
                       const char *to_login, const char *to_jid,
                       const char *msg, const char *answer_id);


/* Sent Queries */

void xmpp_print_number_of_occupants(int wfs, char *room);
void xmpp_bind(const char *resource);
void xmpp_iq_session(void);
void xmpp_iq_account(void);
void xmpp_iq_create_profile(void);
void xmpp_iq_get_account_profiles(void);
void xmpp_iq_get_master_server(const char *channel);
void xmpp_iq_join_channel(void);
void xmpp_iq_peer_status_update(const char *to_jid);
void xmpp_promote_room_master(const char *nickname);
void xmpp_iq_player_status(int status);
void xmpp_iq_gameroom_leave(void);

/* Received Queries */

# define XMPP_REGISTER_QUERY_HDLR()                 \
    do {                                            \
        void xmpp_iq_ping_r(void);                  \
        void xmpp_iq_friend_list_r(void);           \
        void xmpp_iq_peer_status_update_r(void);    \
        void xmpp_message_r(void);                  \
        void xmpp_iq_gameroom_sync_r(void);         \
        void xmpp_iq_invitation_request_r(void);    \
        void xmpp_iq_follow_send_r(void);           \
        void xmpp_iq_sync_notifications_r(void);    \
                                                    \
        xmpp_iq_ping_r();                           \
        xmpp_iq_friend_list_r();                    \
        xmpp_iq_peer_status_update_r();             \
        xmpp_message_r();                           \
        xmpp_iq_gameroom_sync_r();                  \
        xmpp_iq_invitation_request_r();             \
        xmpp_iq_follow_send_r();                    \
        xmpp_iq_sync_notifications_r();             \
    } while (0);

#endif /* !WB_XMPP_WF_H */
