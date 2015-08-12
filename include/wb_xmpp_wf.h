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

#include <wb_xmpp.h>
#include <wb_friend.h>
#include <wb_clanmate.h>

char *wf_get_query_content(const char *msg);


void xmpp_send_message(const char *to_login, const char *to_jid,
                       const char *msg);

void xmpp_ack_message(const char *from_login, const char *from_jid,
                       const char *msg, const char *answer_id);


/* Sent Queries */

void xmpp_print_number_of_occupants(int wfs, char *room);
void xmpp_bind(const char *resource);
void xmpp_iq_session(void);
void xmpp_iq_account(void);
void xmpp_iq_create_profile(void);
void xmpp_iq_confirm_notification(const char *notif);
void xmpp_iq_get_account_profiles(void);
void xmpp_iq_get_master_server(const char *channel);
typedef void (*f_join_channel_cb)(void *args);
void xmpp_iq_join_channel(const char *channel, f_join_channel_cb f, void *args);
void xmpp_iq_peer_status_update(const char *to_jid);
void xmpp_iq_peer_status_update_friend(struct friend *f, void *args);
void xmpp_iq_peer_clan_member_update_clanmate(struct clanmate *f, void *args);
void xmpp_iq_peer_clan_member_update(const char *to_jid);
typedef void (*f_peer_player_info_cb)(const char *info, void *args);
void xmpp_iq_peer_player_info(const char *online_id,
                              f_peer_player_info_cb f, void *args);
void xmpp_promote_room_master(const char *nickname);
void xmpp_iq_player_status(int status);
typedef void (*f_profile_info_get_status_cb)(const char *info, void *args);
void xmpp_iq_profile_info_get_status(const char *nickname,
                                     f_profile_info_get_status_cb f,
                                     void *args);
void xmpp_iq_gameroom_leave(void);
void xmpp_iq_gameroom_join(const char *channel, const char *room_id);
typedef void (*f_gameroom_open_cb)(const char *room_id, void *args);
void xmpp_iq_gameroom_open(const char *mission_key,
                           f_gameroom_open_cb fun, void *args);
void xmpp_iq_remove_friend(const char *nickname);


typedef void (*f_list_cb)(struct list *, void *args);
void xmpp_iq_missions_get_list(f_list_cb fun, void *args);
void xmpp_iq_gameroom_setplayer(int team_id, int room_status, int class_id,
                                f_id_callback cb, void *args);
void xmpp_iq_invitation_send(const char *nickname, int is_follow,
                             f_query_callback cb, void *args);

/* Received Queries */

# define XMPP_WF_REGISTER_QUERY_HDLR()              \
    do {                                            \
        void xmpp_iq_friend_list_r(void);           \
        void xmpp_iq_peer_status_update_r(void);    \
        void xmpp_message_r(void);                  \
        void xmpp_iq_gameroom_sync_r(void);         \
        void xmpp_iq_gameroom_on_kicked_r(void);    \
        void xmpp_iq_gameroom_offer_r(void);        \
        void xmpp_iq_invitation_request_r(void);    \
        void xmpp_iq_follow_send_r(void);           \
        void xmpp_iq_sync_notifications_r(void);    \
        void xmpp_iq_p2p_ping_r(void);              \
        void xmpp_iq_preinvite_invite_r(void);      \
        void xmpp_iq_clan_info_r(void);             \
        void xmpp_iq_peer_clan_member_update_r(void);\
        void xmpp_iq_peer_player_info_r(void);      \
                                                    \
        xmpp_iq_friend_list_r();                    \
        xmpp_iq_peer_status_update_r();             \
        xmpp_message_r();                           \
        xmpp_iq_gameroom_sync_r();                  \
        xmpp_iq_gameroom_on_kicked_r();             \
        xmpp_iq_gameroom_offer_r();                 \
        xmpp_iq_invitation_request_r();             \
        xmpp_iq_follow_send_r();                    \
        xmpp_iq_sync_notifications_r();             \
        xmpp_iq_p2p_ping_r();                       \
        xmpp_iq_preinvite_invite_r();               \
        xmpp_iq_clan_info_r();                      \
        xmpp_iq_peer_clan_member_update_r();        \
        xmpp_iq_peer_player_info_r();               \
    } while (0);

#endif /* !WB_XMPP_WF_H */
