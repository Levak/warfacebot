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

#ifndef WB_XMPP_WF_H
# define WB_XMPP_WF_H

#include <wb_xmpp.h>
#include <wb_friend.h>
#include <wb_clanmate.h>
#include <wb_gameroom.h>

# include <wb_cvar.h>
# include <wb_session.h>

# define JID_K01 JID(session.online.jid.k01)

# define JID_ANY_MS JID(session.online.jid.ms)

# define JID_MS(Res) &((const struct jid) {     \
            .node = "masterserver",             \
            .domain = session.online.jid.host,  \
            .resource = (char *) (Res)          \
        })

# define JID_ROOM(Id) &((const struct jid) {    \
            .node = (char *) (Id),              \
            .domain = session.online.jid.muc,   \
            .resource = NULL                    \
        })

enum notif_type
{
    NOTIF_MISSION_PERF          = 2,
    NOTIF_ACHIEVEMENT           = 4,
    NOTIF_CUSTOM_MESSAGE        = 8,
    NOTIF_CLAN_INVITE           = 16,
    NOTIF_CLAN_INVITE_RESULT    = 32,
    NOTIF_FRIEND_REQUEST        = 64,
    NOTIF_FRIEND_REQUEST_RESULT = 128,
    NOTIF_GIVE_ITEM             = 256,
    NOTIF_ANNOUNCEMENT          = 512,
    NOTIF_CONTRACT              = 1024,
    NOTIF_GIVE_MONEY            = 2048,
    NOTIF_ITEM_UNVAILABLE       = 4096,
    NOTIF_GIVE_RANDOM_BOX       = 8192,
    NOTIF_ITEM_UNLOCKED         = 32768,
    NOTIF_NEW_RANK              = 131072,
    NOTIF_MESSAGE               = 262144,
    NOTIF_UNLOCK_MISSION        = 1048576,
    NOTIF_DELETED_ITEM          = 2097152,
};

enum notif_result
{
    NOTIF_ACCEPT = 0,
    NOTIF_REFUSE = 1,
};

char *wf_get_query_content(const char *msg);

char *wf_compress_query(const char *iq);
char *wf_decompress_query(const char *iq);

void xmpp_connect(const char *login, const char *pass);
void xmpp_close(void);


void xmpp_send_message(const char *to_login, const char *to_jid,
                       const char *msg);

void xmpp_ack_message(const char *from_login, const char *from_jid,
                       const char *msg, const char *answer_id);


/* Sent Queries */

void xmpp_print_number_of_occupants(int wfs, const char *room);
void xmpp_iq_account(const char *login, const char *password);
void xmpp_iq_create_profile(void);
void xmpp_iq_confirm_notification(const char *notif);
void xmpp_iq_get_account_profiles(void);

typedef void (*f_get_master_server_cb)(const char *resource,
                                       int load_index,
                                       void *args);
void xmpp_iq_get_master_server(unsigned int rank,
                               const char *channel_type,
                               f_get_master_server_cb cb,
                               void *args);

typedef void (*f_get_master_servers_cb)(struct list *masterserver,
                                        void *args);
void xmpp_iq_get_master_servers(f_get_master_servers_cb cb,
                                void *args);

typedef void (*f_join_channel_cb)(void *args);
void xmpp_iq_join_channel(const char *channel, f_join_channel_cb f, void *args);

typedef void (*f_resync_profile_cb)(void *args);
void xmpp_iq_resync_profile(f_resync_profile_cb f, void *args);

void xmpp_iq_peer_status_update(const struct friend *f);
void xmpp_iq_peer_status_update_friend(const struct friend *f, void *args);
void xmpp_iq_peer_clan_member_update_clanmate(const struct clanmate *c, void *args);
void xmpp_iq_peer_clan_member_update(const struct clanmate *c);
typedef void (*f_peer_player_info_cb)(const char *info, void *args);
void xmpp_iq_peer_player_info(const char *online_id,
                              f_peer_player_info_cb f, void *args);

typedef void (*f_gameroom_promote_to_host_cb)(void *args);
void xmpp_iq_gameroom_promote_to_host(const char *profile_id,
                                      f_gameroom_promote_to_host_cb cb,
                                      void *args);

void xmpp_iq_player_status(int status);
typedef void (*f_profile_info_get_status_cb)(const char *info, void *args);
void xmpp_iq_profile_info_get_status(const char *nickname,
                                     f_profile_info_get_status_cb f,
                                     void *args);
void xmpp_iq_gameroom_leave(void);

enum join_reason
{
    JOIN_ROOM_BROWSER = 0,
    JOIN_MATCH_MAKING = 1,
    JOIN_INVITE = 2,
    JOIN_FOLLOW = 3,
};

void xmpp_iq_gameroom_join(const char *channel,
                           const char *room_id,
                           const char *group_id,
                           enum join_reason reason);

typedef void (*f_gameroom_open_cb)(const char *room_id, void *args);

void xmpp_iq_gameroom_open(const char *mission_key, enum room_type type,
                           f_gameroom_open_cb fun, void *args);

typedef void (*f_gameroom_quickplay_cb)(void *args);

void xmpp_iq_gameroom_quickplay(const char *uid,
                                const char *mission_key,
                                enum room_type type,
                                const char *game_mode,
                                int channel_switches,
                                f_gameroom_quickplay_cb cb,
                                void *args);

typedef void (*f_gameroom_quickplay_cancel_cb)(void *args);

void xmpp_iq_gameroom_quickplay_cancel(f_gameroom_quickplay_cancel_cb cb,
                                       void *args);

typedef void (*f_gameroom_get_cb)(struct list *room_list, void *args);

void xmpp_iq_gameroom_get(const char *channel,
                          enum room_type type,
                          f_gameroom_get_cb cb,
                          void *args);

typedef void (*f_preinvite_invite_cb)(void *args);

void xmpp_iq_preinvite_invite(const char *online_id,
                              const char *uid,
                              f_preinvite_invite_cb cb,
                              void *args);

typedef void (*f_preinvite_cancel_cb)(void *args);

enum preinvite_cancel_reason
{
    PREINVITE_CANCELED_BY_CLIENT = 0,
    PREINVITE_CANCELED_BY_MASTER = 1,
    PREINVITE_EXPIRED = 2,
    PREINVITE_TIMEOUT = 3,
};

void xmpp_iq_preinvite_cancel(const char *online_id,
                              const char *uid,
                              enum preinvite_cancel_reason reason,
                              f_preinvite_cancel_cb cb,
                              void *args);

void xmpp_iq_remove_friend(const char *nickname);


typedef void (*f_missions_get_list_cb)(struct list *l,
                                       int hash,
                                       int content_hash,
                                       void *args);

void xmpp_iq_missions_get_list(f_missions_get_list_cb fun,
                               void *args);

typedef void (*f_gameroom_setplayer_cb)(void *args);
void xmpp_iq_gameroom_setplayer(int team_id, int room_status, int class_id,
                                f_gameroom_setplayer_cb cb, void *args);

typedef void (*f_gameroom_askserver_cb)(void *args);
void xmpp_iq_gameroom_askserver(f_gameroom_askserver_cb cb, void *args);

void xmpp_iq_get_player_stats(f_id_callback cb, void *args);

void xmpp_iq_get_achievements(const char *profile_id,
                              f_id_callback cb, void *args);

enum invitation_result
{
    INVIT_ACCEPTED = 0,
    INVIT_REJECTED = 1,
    INVIT_AUTOREJECT = 2,
    INVIT_PENDING = 3,
    INVIT_DUPLICATE = 4,
    INVIT_USER_OFFLINE = 5,
    INVIT_USER_NOT_IN_ROOM = 6,
    INVIT_LIMIT_REACHED = 7,
    INVIT_SERVICE_ERROR = 8,
    INVIT_EXPIRED = 9,
    INVIT_DUPLICATED_FOLLOW = 10,
    INVIT_INVALID_TARGET = 11,
    INVIT_MISSION_RESTRICTED = 12,
    INVIT_RANK_RESTRICTED = 13,
    INVIT_FULL_ROOM = 14,
    INVIT_KICKED = 15,
    INVIT_BUILD_TYPE = 16,
    INVIT_PRIVATE_ROOM = 17,
    INVIT_NOT_IN_CLAN = 18,
    INVIT_NOT_IN_CW = 19,
    INVIT_RATING = 23,
    INVIT_ERROR = 0xFF,
};

typedef void (*f_invitation_result_cb)(const char *channel,
                                       enum invitation_result r,
                                       void *args);

void invitation_register(const char *nickname,
                         char is_follow,
                         f_invitation_result_cb cb,
                         void *args);

int invitation_complete(const char *nickname,
                        const char *channel,
                        enum invitation_result r,
                        int is_follow);

void xmpp_iq_follow_send(const char *nickname,
                         const char *online_id,
                         f_invitation_result_cb cb, void *args);

void xmpp_iq_invitation_send(const char *nickname, int is_follow,
                             f_invitation_result_cb cb, void *args);

typedef void (*f_gameroom_setinfo_cb)(void *args);
void xmpp_iq_gameroom_setinfo(const char *mission_key,
                              f_gameroom_setinfo_cb cb, void *args);

typedef void (*f_gameroom_setname_cb)(void *args);
void xmpp_iq_gameroom_setname(const char *room_name,
                              f_gameroom_setname_cb cb, void *args);

enum kick_reason
{
    KICK_NOREASON = 0,
    KICK_MASTER = 1,
    KICK_INACTIVITY = 2,
    KICK_VOTE = 3,
    KICK_RANK = 6,
    KICK_CLAN_ROOM = 7,
    KICK_CHEATING = 8,
    KICK_GAME_VERSION = 9,
    KICK_NOTOKEN = 10,
    KICK_MATCHMAKING = 11,
    KICK_RATING_END = 12,
    KICK_LATENCY = 13,
};

void xmpp_iq_gameroom_kick(unsigned int profile_id,
                           f_id_callback cb, void *args);

typedef void (*f_send_invitation_cb)(void *args);
void xmpp_iq_send_invitation(const char *nickname,
                             enum notif_type type,
                             f_send_invitation_cb cb, void *args);

enum pvp_mode
{
    PVP_DEFAULT = 0,
    PVP_PRIVATE = 1 << 0,
    PVP_FRIENDLY_FIRE = 1 << 1,
    PVP_ENEMY_OUTLINES = 1 << 2,
    PVP_AUTOBALANCE = 1 << 3,
    PVP_DEADCHAT = 1 << 4,
    PVP_ALLOWJOIN = 1 << 5
};

typedef void (*f_gameroom_update_pvp_cb)(void *args);
void xmpp_iq_gameroom_update_pvp(const char *mission_key, enum pvp_mode flags,
                                 int max_players, int inventory_slot,
                                 f_gameroom_update_pvp_cb cb, void *args);

typedef void (*f_get_last_seen_date_cb)(const char *profile_id,
                                       unsigned int timestamp,
                                       void *args);
void xmpp_iq_get_last_seen_date(const char *profile_id,
                                f_get_last_seen_date_cb cb,
                                void *args);

void gameroom_sync_init(void);
void gameroom_sync(const char *data);
void gameroom_sync_free(void);

enum sponsor_type
{
    SPONSOR_WEAPON    = 0,
    SPONSOR_OUTFIT    = 1,
    SPONSOR_EQUIPMENT = 2,
};

typedef void (*f_shop_get_offers_cb)(const struct querycache *cache,
                                     void *args);
void querycache_shop_get_offers_init(void);
void querycache_shop_get_offers_free(void);
void xmpp_iq_shop_get_offers(f_shop_get_offers_cb cb,
                             void *args);

typedef void (*f_items_cb)(const struct querycache *cache,
                           void *args);
void querycache_items_init(void);
void querycache_items_free(void);
void xmpp_iq_items(f_items_cb cb, void *args);

typedef void (*f_quickplay_maplist_cb)(const struct querycache *cache,
                           void *args);
void querycache_quickplay_maplist_init(void);
void querycache_quickplay_maplist_free(void);
void xmpp_iq_quickplay_maplist(f_quickplay_maplist_cb cb, void *args);

typedef void (*f_get_configs_cb)(const struct querycache *cache,
                                 void *args);
void querycache_get_configs_init(void);
void querycache_get_configs_free(void);
void xmpp_iq_get_configs(f_get_configs_cb cb, void *args);

/* Received Queries */

# define XMPP_WF_REGISTER_QUERY_HDLR()              \
    do {                                            \
        void xmpp_iq_friend_list_r(void);           \
        void xmpp_iq_peer_status_update_r(void);    \
        void xmpp_message_r(void);                  \
        void xmpp_iq_gameroom_sync_r(void);         \
        void xmpp_iq_gameroom_on_kicked_r(void);    \
        void xmpp_iq_gameroom_on_expired_r(void);   \
        void xmpp_iq_gameroom_offer_r(void);        \
        void xmpp_iq_gameroom_loosemaster_r(void);  \
        void xmpp_iq_invitation_request_r(void);    \
        void xmpp_iq_invitation_result_r(void);     \
        void xmpp_iq_follow_send_r(void);           \
        void xmpp_iq_sync_notifications_r(void);    \
        void xmpp_iq_notification_broadcast_r(void);\
        void xmpp_iq_p2p_ping_r(void);              \
        void xmpp_iq_preinvite_invite_r(void);      \
        void xmpp_iq_clan_info_r(void);             \
        void xmpp_iq_peer_clan_member_update_r(void);\
        void xmpp_iq_peer_player_info_r(void);      \
        void xmpp_iq_shop_sync_consummables_r(void);\
        void xmpp_iq_broadcast_session_result_r(void);\
        void xmpp_iq_sponsor_info_updated_r(void);  \
        void xmpp_iq_clan_members_updated_r(void);  \
        void xmpp_iq_clan_masterbanner_update_r(void);\
        void xmpp_iq_update_cry_money_r(void);      \
        void xmpp_iq_gameroom_quickplay_started_r(void);\
        void xmpp_iq_gameroom_quickplay_succeeded_r(void);\
        void xmpp_iq_gameroom_quickplay_canceled_r(void);\
        void xmpp_iq_preinvite_response_r(void);    \
        void xmpp_iq_preinvite_cancel_r(void);      \
        void xmpp_iq_autorotate_r(void);            \
                                                    \
        xmpp_iq_friend_list_r();                    \
        xmpp_iq_peer_status_update_r();             \
        xmpp_message_r();                           \
        xmpp_iq_gameroom_sync_r();                  \
        xmpp_iq_gameroom_on_kicked_r();             \
        xmpp_iq_gameroom_on_expired_r();            \
        xmpp_iq_gameroom_offer_r();                 \
        xmpp_iq_gameroom_loosemaster_r();           \
        xmpp_iq_invitation_request_r();             \
        xmpp_iq_invitation_result_r();              \
        xmpp_iq_follow_send_r();                    \
        xmpp_iq_sync_notifications_r();             \
        xmpp_iq_notification_broadcast_r();         \
        xmpp_iq_p2p_ping_r();                       \
        xmpp_iq_preinvite_invite_r();               \
        xmpp_iq_clan_info_r();                      \
        xmpp_iq_peer_clan_member_update_r();        \
        xmpp_iq_peer_player_info_r();               \
        xmpp_iq_shop_sync_consummables_r();         \
        xmpp_iq_broadcast_session_result_r();       \
        xmpp_iq_sponsor_info_updated_r();           \
        xmpp_iq_clan_members_updated_r();           \
        xmpp_iq_clan_masterbanner_update_r();       \
        xmpp_iq_update_cry_money_r();               \
        xmpp_iq_gameroom_quickplay_started_r();     \
        xmpp_iq_gameroom_quickplay_succeeded_r();   \
        xmpp_iq_gameroom_quickplay_canceled_r();    \
        xmpp_iq_preinvite_response_r();             \
        xmpp_iq_preinvite_cancel_r();               \
        xmpp_iq_autorotate_r();                     \
    } while (0);

#endif /* !WB_XMPP_WF_H */
