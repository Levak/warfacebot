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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#ifdef DEBUG
# include <readline/readline.h>
# include <readline/history.h>
#endif

#include <wb_game.h>
#include <wb_stream.h>
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>
#include <wb_tools.h>
#include <wb_session.h>
#include <wb_cmd.h>

/** THEADS **/

#ifdef __MINGW32__
# include <windows.h>
# define sleep(x) Sleep(x)
#endif

#ifdef DEBUG
void *thread_readline(void *varg)
{
    int wfs = session.wfs;

    using_history();

    do {
        char *buff_readline = readline("CMD# ");

        if (buff_readline == NULL)
        {
            if (session.active)
            {
                xmpp_iq_player_status(STATUS_OFFLINE);
                xmpp_close(session.wfs);
            }

            free(buff_readline);
            break;
        }

        int buff_size = strlen(buff_readline);

        if (buff_size <= 1)
            flush_stream(wfs);
        else
        {
            add_history(buff_readline);

            if (strstr(buff_readline, "remove"))
            {
                char *nickname = strchr(buff_readline, ' ');

                if (nickname != NULL)
                    cmd_remove_friend(nickname + 1);
            }

            else if (strstr(buff_readline, "whois"))
            {
                char *nickname = strchr(buff_readline, ' ');

                if (nickname != NULL)
                    cmd_whois(nickname + 1, NULL, NULL);
            }

            else if (strstr(buff_readline, "missions"))
            {
                cmd_missions(NULL, NULL);
            }

            else if (strstr(buff_readline, "say"))
            {
                char *message = strchr(buff_readline, ' ');

                if (message != NULL)
                    cmd_say(message + 1);
            }

            else if (strstr(buff_readline, "leave"))
            {
                cmd_leave();
            }

            else
                send_stream(wfs, buff_readline, buff_size);
            sleep(1);
        }

        free(buff_readline);
    } while (session.active);

    session.active = 0;
    printf("Closed readline\n");
    pthread_exit(NULL);
}
#endif

#ifdef STAT_BOT
static void print_number_of_players_cb(const char *msg, void *args)
{
    unsigned int count_all = 0;
    unsigned int count_pvp = 0;
    unsigned int count_pve = 0;

    const char *m = msg;
    while ((m = strstr(m, "<item")))
    {
        /* Extract room jid */
        char *rjid = get_info(m, "jid='", "'", NULL);

        /* It's not a clan room */
        if (!strstr(rjid, "clan"))
        {
            unsigned int count = get_info_int(m, "(", ")", NULL);

            if (strstr(rjid, "pve"))
                count_pve += count;
            else if (strstr(rjid, "pvp"))
                count_pvp += count;
            count_all += count;
        }

        free(rjid);
        ++m;
    }

    printf("%u,%u,%u,%u\n", (unsigned) time(NULL), count_all, count_pve, count_pvp);
    fflush(stdout);
}

void *thread_stats(void *varg)
{
    int wfs = session.wfs;

    idh_register((t_uid *) "stats", 1, &print_number_of_players_cb, NULL);

    sleep(3);

    do {
        send_stream_ascii(wfs,
                          "<iq to='conference.warface' type='get' id='stats'>"
                          " <query xmlns='http://jabber.org/protocol/disco#items'/>"
                          "</iq>");
        flush_stream(wfs);
        sleep(5);
    } while (session.active);

    printf("Closed stats\n");
    pthread_exit(NULL);
}
#endif

void *thread_dispatch(void *vargs)
{
    XMPP_REGISTER_QUERY_HDLR();
    XMPP_WF_REGISTER_QUERY_HDLR();

    do {
        char *msg = read_stream_keep(session.wfs);

        if (msg == NULL || strlen(msg) <= 0)
        {
            if (session.active)
            {
                xmpp_iq_player_status(STATUS_OFFLINE);
                xmpp_close(session.wfs);
            }

            break;
        }

        char *msg_id = get_msg_id(msg);

        /* Look if the ID is registered in the query callback handler */
        if (idh_handle(msg_id, msg))
        {
            /* Nothing to be done here */
        }
        else /* Unhandled ID */
        {
            char *stanza = get_query_tag_name(msg);

            /* Look if tagname is registered in the stanza callback handler */
            if (qh_handle(stanza, msg_id, msg))
            {
                /* Nothing to be done here */
            }
            else /* Unhandled stanza */
            {
                /* TODO: print it for readline ? */
            }

            free(stanza);
        }

        free(msg);
        free(msg_id);

    } while (session.active);

    session.active = 0;
    printf("Closed idle\n");
    pthread_exit(NULL);
}

void idle(void)
{
#if defined (STAT_BOT) || defined (DEBUG)

    pthread_t thread_dl;
    void * (*thread_func)(void *);

# ifdef STAT_BOT
    thread_func = &thread_stats;
# elif defined (DEBUG)
    thread_func = &thread_readline;
# endif

    if (pthread_create(&thread_dl, NULL, thread_func, NULL) == -1)
        perror("pthread_create");

    pthread_detach(thread_dl);
#endif
}

int main(int argc, char *argv[])
{
    if (argc <= 2)
    {
        fprintf(stderr, "USAGE: ./wb token online_id [eu/na/tr/vn/ru]\n");
        return 2;
    }

    char *token = argv[1];
    char *online_id = argv[2];
    enum e_server server = SERVER_EU;

    if (argc > 3)
    {
        if (strcmp(argv[3], "eu") == 0)
            server = SERVER_EU;
        else if (strcmp(argv[3], "na") == 0)
            server = SERVER_NA;
        else if (strcmp(argv[3], "tr") == 0)
            server = SERVER_TR;
        else if (strcmp(argv[3], "ru") == 0)
          server = SERVER_RU;
/*        else if (strcmp(argv[3], "br") == 0)
          server = SERVER_BR;*/
/*        else if (strcmp(argv[3], "cn") == 0)
          server = SERVER_CN;*/
        else if (strcmp(argv[3], "vn") == 0)
            server = SERVER_VN;
        else
            fprintf(stderr, "Unknown server, falling back on EU.\n");
    }

    game_set(server);

    int wfs = connect_wf(game_xmpp_server_get(), 5222);

    session_init(wfs, online_id);

    xmpp_connect(wfs, token, session.online_id);

    pthread_t thread_di;
    if(pthread_create(&thread_di, NULL, thread_dispatch, NULL) == -1)
        perror("pthread_create");

    xmpp_bind("GameClient");

    idle();

    if (session.active)
        pthread_join(thread_di, NULL);

    session_free();
    printf("Warface Bot closed!");
    return 0;
}

/* Unresolved Queries:
-----------------------

aas_notify_playtime
autorotate
broadcast_session_result
confirm_notification
expire_profile_items profile_idle class_id time_played
external_shop_confirm_query supplierId orderId
external_shop_refund_query supplierId orderId
follow_send
invitation_accept
invitation_request
invitation_result
invitation_send
lobbychat_getchannelid channel
map_voting_finished
map_voting_started
map_voting_state
map_voting_vote mission_uid
mission_load
mission_load_failed
mission_unload
mission_update
notification_broadcast
on_voting_finished
on_voting_started
on_voting_vote
persistent_settings_get
persistent_settings_set
preinvite_invite
preinvte_response
quickplay_maplist received size
remove_friend target=nickname
resync_profile
send_invitation target type
set_banner
shop_get_offers
shop_buy_offer supplier_id offer_id
shop_buy_multiple_offer supplier_id/offer id
shop_buy_external_offer supplier_id offer_id
shop_buy_multiple_external_offer supplier_id/offer id
sponsor_info_updated
srv_player_kicked
sync_notifications
tutorial_status

validate_payer_info
voting_start target
voting_vote answer
set_reward_info session_id difficulty isPvE isClanWar mission_id incomplete_session session_time session_kill_counter winning_team_id passed_sublevels_counter passed_checkpoints_counter secondary_objectives_completed max_session_score/players_performance/stat id value
customhead_reset default_head
========= */

/* Results :
------------

friend_list
clan_info
update_cry_money
peer_clan_member_update
peer_status_update

========= */

/* Queries for ms.warface :
---------------------------

channel_logout X
items
gameroom_setgameprogress room_id game_progress
gameroom_askserver server
gameroom_join room_id team_id group_id status class_id
gameroom_get room_type='14' size='108' received='0' cancelled='0' token='0'
gameroom_kick target_id
gameroom_leave
gameroom_loosemaster
gameroom_offer_response
gameroom_on_expired
gameroom_on_kicked
gameroom_open [mission_data] room_name team_id status class_id room_type private mission friendly_fire enemy_outlines auto_team_balance dead_can_chat join_in_the_process max_player inventory_slot class_riffleman(enabled) [...] / session?
gameroom_promote_to_host new_host_profile_id
gameroom_quickplay room_type room_name mission_id mission_type status team_id class_id missions_hash content_hash channel_switches? timestamp? uid? group?// session
gameroom_quickplay_cancel
gameroom_quickplay_canceled
gameroom_quickplay_failed
gameroom_quickplay_started
gameroom_quickplay_succeeded
gameroom_setinfo by_mission_key mission_key data
gameroom_setname room_name
gameroom_setplayer team_id status class_id
gameroom_setprivatestatus private
gameroom_setteamcolor
gameroom_switchteams
gameroom_sync
gameroom_update_pvp by_mission_key mission_key [data] private
generic_telemetry X
telemetry_stream session_id packet_id finalize
get_account_profiles version='1.1.1.3522' user_id='xxxxxxxx' token='$account_token'
get_achievements/achievement profile_id='xxxx'
get_configs
get_last_seen_date profile_id='xxxxxx'
get_player_stats
ingame_event profile_id event_name [data?key/value]// error
missions_get_list
validate_player_info online_id nickname banner_badge banner_mark banner_stripe experience clan_name clan_role clan_position clan_points clan_member_since
class_presence

======== */


/* Queries for masterserver@warface/pve_12 :
--------------------------------------------

telemetry_getleaderboard limit='10' // Removed ?
admin_cmd command='' args='' // kick ban unban mute unmute
get_contracts profile_id // doesn't work with other profile_id
get_cry_money
get_profile_performance
get_storage_items
session_join
session_join_dediclient
unbind_server_info server
extend_item item_id supplier_id offer_id
get_expired_items
update_achievements[cheat_mode=1]/achievement profile_id='xxxx'/chunk achievement_id='53' progress='15813349' completion_time='0'
update_contracts
abuse_report target type comment
peer_player_info to online_id@warface/GameClient
clan_info_sync
clan_list
getprofile session_id id
setcharacter gender(female/male) height fatness current_class
setcurrentclass current_class
ui_user_choice/choice choice_id choice_result

======== */


/* Queries for k01.warface :
----------------------------

account login=userid [password=token]
create_profile version='1.1.1.3522' token='$account_token' nickname='xxxxx' user_id='xxxxxxxx' resource='pve_12'
get_master_server rank channel resource used_resources is_quickplay
get_master_servers
join_channel version='1.1.1.3522' token='$account_token' profile_id='xxxx' user_id='xxxxxxxx' resource='pve_12' build_type='--release'
switch_channel version='1.1.1.3522' token='$account_token' profile_id='xxxx' user_id='xxxxxxxx' resource='pve_12' build_type='--release'
player_status prev_status='1' new_status='9' to='pve_12' // Bitfield 0:Online 1:??? 2:AFK 3:Lobby 4:Room 5:Playing 6:Shop 7:Inventory
profile_info_get_status nickname='xxxxxx'
iq/user_status[k01:presence] invalid

======== */
