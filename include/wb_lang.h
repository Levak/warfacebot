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

#ifndef WB_LANG
# define WB_LANG

# include <wb_cvar.h>

# define LANG_LIST                              \
    XLANG(default_room_name)                    \
    XLANG(wb_closing)                           \
                                                \
    XLANG(error)                                \
    XLANG(error_authentication)                 \
    XLANG(error_parse_error)                    \
    XLANG(error_file_notfound)                  \
    XLANG(error_required_option)                \
    XLANG(error_required_cvar)                  \
    XLANG(error_create_directory)               \
    XLANG(error_loading_file)                   \
    XLANG(error_write_file)                     \
    XLANG(error_read_file)                      \
    XLANG(error_no_channel_type)                \
    XLANG(error_no_user)                        \
    XLANG(error_user_not_connected)             \
    XLANG(error_no_map)                         \
    XLANG(error_command_not_found)              \
    XLANG(error_command_usage)                  \
    XLANG(error_set_cvar)                       \
    XLANG(error_get_info)                       \
                                                \
    XLANG(error_get_master_server)              \
    XLANG(error_qos_limit)                      \
    XLANG(error_invalid_channel)                \
    XLANG(error_invalid_login)                  \
    XLANG(error_invalid_nickname)               \
    XLANG(error_game_version)                   \
    XLANG(error_create_profile)                 \
    XLANG(error_not_in_a_clan)                  \
    XLANG(error_already_in_a_clan)              \
    XLANG(error_friendlist_full)                \
    XLANG(error_send_invitation)                \
    XLANG(error_unknown_mission)                \
    XLANG(error_room_started)                   \
    XLANG(error_gameroom_setinfo)               \
    XLANG(error_get_account_profiles)           \
    XLANG(error_already_logged_in)              \
    XLANG(error_timeout)                        \
    XLANG(error_gameroom_open)                  \
    XLANG(error_expired_missions)               \
    XLANG(error_invalid_mission)                \
    XLANG(error_not_master)                     \
    XLANG(error_rank_restricted)                \
    XLANG(error_invalid_room_name)              \
    XLANG(error_banned)                         \
    XLANG(error_invalid_profile)                \
    XLANG(error_join_channel)                   \
    XLANG(error_gameroom_get)                   \
    XLANG(error_gameroom_askserver)             \
    XLANG(error_not_balanced)                   \
    XLANG(error_gameroom_quickplay)             \
    XLANG(error_not_rating_season)              \
    XLANG(error_account)                        \
    XLANG(error_follow)                         \
    XLANG(error_invite)                         \
    XLANG(error_leave_room)                     \
    XLANG(error_join_room)                      \
                                                \
    XLANG(quickplay_canceled)                   \
    XLANG(quickplay_no_pending)                 \
    XLANG(quickplay_in_progress)                \
    XLANG(quickplay_started)                    \
    XLANG(quickplay_done)                       \
    XLANG(preinvite_rejected)                   \
    XLANG(preinvite_accepted)                   \
    XLANG(preinvite_canceled)                   \
    XLANG(preinvite_canceled_by_master)         \
    XLANG(preinvite_expired)                    \
                                                \
    XLANG(invite_rejected)                      \
    XLANG(invite_pending)                       \
    XLANG(invite_autoreject)                    \
    XLANG(invite_duplicate_follow)              \
    XLANG(invite_duplicate)                     \
    XLANG(invite_user_offline)                  \
    XLANG(invite_user_not_in_room)              \
    XLANG(invite_not_in_room)                   \
    XLANG(invite_expired)                       \
    XLANG(invite_invalid_target)                \
    XLANG(invite_mission_restricted)            \
    XLANG(invite_rank_restricted)               \
    XLANG(invite_full_room)                     \
    XLANG(invite_kicked)                        \
    XLANG(invite_private_room)                  \
    XLANG(invite_not_in_cw)                     \
    XLANG(invite_rating)                        \
    XLANG(invite_accepted)                      \
    XLANG(follow_accepted)                      \
                                                \
    XLANG(console_friends)                      \
    XLANG(console_friend)                       \
    XLANG(console_removed_friend)               \
    XLANG(console_clanmates)                    \
    XLANG(console_clanmate)                     \
    XLANG(console_last_seen)                    \
    XLANG(console_missions_time)                \
    XLANG(console_missions_score)               \
    XLANG(console_quickplay_map_required)       \
    XLANG(console_quickplay_nick_required)      \
    XLANG(console_randombox_available)          \
    XLANG(console_randombox_invalid_amount)     \
    XLANG(console_randombox_required_amount)    \
    XLANG(console_randombox_unknown)            \
    XLANG(console_sponsor)                      \
    XLANG(console_sponsor_weapon)               \
    XLANG(console_sponsor_outfit)               \
    XLANG(console_sponsor_equipment)            \
    XLANG(console_sponsor_unknown)              \
    XLANG(console_sponsor_should_be)            \
    XLANG(console_stay)                         \
    XLANG(console_stats_total)                  \
                                                \
    XLANG(shop_error_purshase)                  \
    XLANG(shop_error_no_money)                  \
    XLANG(shop_error_timeout)                   \
    XLANG(shop_error_tag)                       \
    XLANG(shop_error_no_item)                   \
    XLANG(shop_error_limit_reached)             \
    XLANG(shop_error_out_of_store)              \
    XLANG(shop_error_restricted)                \
    XLANG(shop_error_not_ready)                 \
                                                \
    XLANG(shop_rb_item)                         \
    XLANG(shop_money_left)                      \
    XLANG(shop_item_permanent)                  \
                                                \
    XLANG(money_game)                           \
    XLANG(money_game_short)                     \
    XLANG(money_crown)                          \
    XLANG(money_crown_short)                    \
    XLANG(money_cry)                            \
    XLANG(money_cry_short)                      \
    XLANG(money_key)                            \
    XLANG(money_key_short)                      \
    XLANG(experience)                           \
    XLANG(experience_short)                     \
    XLANG(rating_points)                        \
    XLANG(nickname)                             \
    XLANG(profile_id)                           \
    XLANG(rank)                                 \
    XLANG(unlocked_items)                       \
                                                \
    XLANG(whisper_last_seen_never)              \
    XLANG(whisper_last_seen_hour)               \
    XLANG(whisper_last_seen_day)                \
    XLANG(whisper_last_seen_month)              \
    XLANG(whisper_last_seen_year)               \
    XLANG(whisper_missions_line)                \
    XLANG(whisper_whois_unknown)                \
    XLANG(whisper_whois_no_country)             \
    XLANG(whisper_whois_1)                      \
    XLANG(whisper_whois_2)                      \
    XLANG(whisper_whois_3)                      \
                                                \
    XLANG(whisper_leave_1)                      \
    XLANG(whisper_leave_2)                      \
    XLANG(whisper_leave_3)                      \
    XLANG(whisper_leave_4)                      \
                                                \
    XLANG(whisper_unready_1)                    \
    XLANG(whisper_unready_2)                    \
    XLANG(whisper_unready_3)                    \
    XLANG(whisper_unready_4)                    \
                                                \
    XLANG(whisper_ready_1)                      \
    XLANG(whisper_ready_2)                      \
    XLANG(whisper_ready_3)                      \
    XLANG(whisper_ready_4)                      \
                                                \
    XLANG(whisper_master_1)                     \
    XLANG(whisper_master_2)                     \
    XLANG(whisper_master_3)                     \
    XLANG(whisper_master_4)                     \
                                                \
    XLANG(whisper_stay_1)                       \
    XLANG(whisper_stay_2)                       \
    XLANG(whisper_stay_3)                       \
    XLANG(whisper_stay_4)                       \
                                                \
    XLANG(whisper_stay_ko_1)                    \
    XLANG(whisper_stay_ko_2)                    \
    XLANG(whisper_stay_ko_3)                    \
    XLANG(whisper_stay_ko_4)                    \
                                                \
    XLANG(whisper_unknown_1)                    \
    XLANG(whisper_unknown_2)                    \
    XLANG(whisper_unknown_3)                    \
    XLANG(whisper_unknown_4)                    \
                                                \
    XLANG(status_afk)                           \
    XLANG(status_tutorial)                      \
    XLANG(status_rating)                        \
    XLANG(status_playing)                       \
    XLANG(status_shop)                          \
    XLANG(status_inventory)                     \
    XLANG(status_room)                          \
    XLANG(status_lobby)                         \
    XLANG(status_online)                        \
    XLANG(status_offline)                       \
                                                \
    XLANG(room_leave)                           \
    XLANG(room_leave_fail)                      \
    XLANG(room_join)                            \
    XLANG(room_join_fail)                       \
                                                \
    XLANG(ping_is_over)                         \
    XLANG(ping_is_alive)                        \
    XLANG(ping_stalling)                        \
                                                \
    XLANG(channel)                              \
    XLANG(channel_joined)                       \
    XLANG(created_profile)                      \
                                                \
    XLANG(notif_confirm_expiration)             \
    XLANG(notif_message)                        \
    XLANG(notif_custom_message)                 \
    XLANG(notif_new_rank)                       \
    XLANG(notif_unlock_mission)                 \
    XLANG(notif_unlock_item)                    \
    XLANG(notif_unlocking)                      \
    XLANG(notif_unlocking_done)                 \
    XLANG(notif_money_given)                    \
    XLANG(notif_item_given)                     \
    XLANG(notif_randombox_given)                \
    XLANG(notif_deleted_item)                   \
    XLANG(notif_friend_request)                 \
    XLANG(notif_friend_request_accepted)        \
    XLANG(notif_friend_request_rejected)        \
    XLANG(notif_clan_invite)                    \
    XLANG(notif_clan_invite_rejected)           \
    XLANG(notif_clan_invite_failed)             \
    XLANG(notif_clan_joined)                    \
    XLANG(notif_clan_left)                      \
    XLANG(notif_postponed)                      \
    XLANG(notif_accepted)                       \
    XLANG(notif_rejected)                       \
    XLANG(notif_room_invitation)                \
    XLANG(notif_room_preinvitation)             \
                                                \
    XLANG(update_profile_status)                \
    XLANG(update_rating_points)                 \
    XLANG(update_consummable)                   \
    XLANG(update_room_master)                   \
    XLANG(update_mission)                       \
    XLANG(update_players)                       \
    XLANG(update_auto_start)                    \
    XLANG(update_auto_start_canceled)           \
    XLANG(gameroom_started)                     \
    XLANG(gameroom_leave)                       \
    XLANG(gameroom_stay)                        \
    XLANG(gameroom_loosemaster)                 \
    XLANG(gameroom_autostart)                   \
    XLANG(gameroom_on_kicked)                   \
                                                \
    XLANG(kick_unknown)                         \
    XLANG(kick_no_reason)                       \
    XLANG(kick_by_master)                       \
    XLANG(kick_inactivity)                      \
    XLANG(kick_by_vote)                         \
    XLANG(kick_rank_too_high)                   \
    XLANG(kick_not_in_cw)                       \
    XLANG(kick_cheating)                        \
    XLANG(kick_game_version)                    \
    XLANG(kick_no_token)                        \
    XLANG(kick_matchmaking)                     \
    XLANG(kick_rating_end)                      \
                                                \
    XLANG(help_add)                             \
    XLANG(help_channel)                         \
    XLANG(help_change)                          \
    XLANG(help_exec)                            \
    XLANG(help_friends)                         \
    XLANG(help_follow)                          \
    XLANG(help_help)                            \
    XLANG(help_invite)                          \
    XLANG(help_last)                            \
    XLANG(help_leave)                           \
    XLANG(help_master)                          \
    XLANG(help_missions)                        \
    XLANG(help_name)                            \
    XLANG(help_open)                            \
    XLANG(help_quickplay)                       \
    XLANG(help_quit)                            \
    XLANG(help_randombox)                       \
    XLANG(help_ready)                           \
    XLANG(help_remove)                          \
    XLANG(help_safe)                            \
    XLANG(help_say)                             \
    XLANG(help_sleep)                           \
    XLANG(help_sponsor)                         \
    XLANG(help_stats)                           \
    XLANG(help_stay)                            \
    XLANG(help_start)                           \
    XLANG(help_switch)                          \
    XLANG(help_unready)                         \
    XLANG(help_whisper)                         \
    XLANG(help_whois)                           \


typedef struct {
    const t_cvar_str value;
    const int is_set;
} s_lang_assoc;

struct lang
{
# define XLANG(Name) const s_lang_assoc Name;
    LANG_LIST
# undef XLANG
};

extern struct lang lang;

char *lang_get(const char *fmt, ...);

# define LANG(Name) lang.Name.value

# define LANG_FMT(Name, ...)                    \
    ((!lang.Name.is_set)                        \
     ? lang.Name.value                          \
     : lang_get(lang.Name.value,                \
                __VA_ARGS__))

#endif /* !WB_LANG */
