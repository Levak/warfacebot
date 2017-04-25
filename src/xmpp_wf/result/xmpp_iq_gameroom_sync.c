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

#include <wb_tools.h>
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>
#include <wb_list.h>
#include <wb_session.h>
#include <wb_cvar.h>
#include <wb_gameroom.h>
#include <wb_log.h>
#include <wb_lang.h>

#include <stdlib.h>

static void xmpp_iq_session_join_cb(const char *msg,
                                    enum xmpp_msg_type type,
                                    void *args)
{
    /* Answer:
       <iq from='masterserver@warface/pve_1' type='result'>
         <query xmlns='urn:cryonline:k01'>
           <session_join room_id='4645701477'
                         server='ded8-lv-lw-eu_64013'
                         hostname='xxx.xxx.xxx.xx' port='64013'
                         local='0' session_id='7921418322'/>
           </query>
       </iq>
    */

    if (msg == NULL)
        return;

    char *data = wf_get_query_content(msg);

    if (data != NULL)
    {
        char *ip = get_info(data, "hostname='", "'", NULL);
        int port = get_info_int(data, "port='", "'", NULL);

        char *server = get_info(data, "server='", "'", NULL);
        char *room_id = get_info(data, "room_id='", "'", NULL);
        char *session_id = get_info(data, "session_id='", "'", NULL);

        if (session_id != NULL)
        {
            int left = 0;

            /* If we should stay in the room */
            if (session.gameroom.leave_timeout > time(NULL)
                || !cvar.wb_leave_on_start
                || cvar.wb_safemaster)
            {
                /* If it is a Quickplay match */
                if (session.gameroom.sync.auto_start.base.revision != 0)
                {
                    xmpp_iq_gameroom_setplayer(session.gameroom.curr_team,
                                               GAMEROOM_UNREADY,
                                               session.profile.curr_class,
                                               NULL, NULL);
                }

                status_set(STATUS_ONLINE | STATUS_PLAYING);
            }
            else
            {
                xmpp_iq_gameroom_leave();
                left = 1;
            }

            xprintf("%s %s "
                    "(IP/PORT/S/RID/SID: %s %d %s %s %s)",
                    LANG(gameroom_started),
                    (left)
                    ? LANG(gameroom_leave)
                    : LANG(gameroom_stay),
                    ip, port, server, room_id, session_id);
        }

        free(ip);
        free(server);
        free(room_id);
        free(session_id);
        free(data);
    }

    {
    }
}

static void xmpp_iq_gameroom_sync_cb(const char *msg_id,
                                     const char *msg,
                                     void *args)
{
    /* Answer:
       <gameroom_sync bcast_receivers='xxx@warface/GameClient,...'>
        <game_room room_id='xxx' room_type='1'>
         <core teams_switched='0' room_name='xxx' private='0' players='5'
               can_start='0' team_balanced='1' min_ready_players='4'
               revision='xxxx'>
          <players>
           <player profile_id='xx' team_id='1' status='0' observer='0'
                   skill='80.000' nickname='xxx' clanName='xxx'
                   class_id='0' online_id='xxx' group_id='xxx'
                   presence='xx' experience='xxx' rank='xxx'
                   banner_badge='xx' banner_mark='xx' banner_stripe='xx'
                   region_id='global'/>
           ...
          </players>
          <playersReserved/>
          <team_colors>
           <team_color id='1' color='4294907157'/>
           <team_color id='2' color='4279655162'/>
          </team_colors>
         </core>
         <regions region_id='global' revision='xxx'/>
         <custom_params friendly_fire='0' enemy_outlines='1'
                        auto_team_balance='0' dead_can_chat='1'
                        join_in_the_process='1' max_players='5'
                        round_limit='0' class_restriction='253'
                        inventory_slot='2113929215' revision='477'/>
         <mission mission_key='xxxxx' no_teams='1' name='xx'
                  setting='xxx' mode='pve' mode_name='xxx'
                  mode_icon='pve_icon' description='xx' image='xx'
                  difficulty='xx' type='xxx' time_of_day='9:06'
                  revision='635'>
          <objectives factor='1'>
           <objective id='0' type='primary'/>
          </objectives>
         </mission>
         <room_master master='xx' revision='416'/>
         <session id='xxx' status='2' game_progress='1'
                  start_time='xxx' revision='939'/>
         <clan_war clan_1='xxxx' clan_2='yyy'/>
        </game_room>
       </gameroom_sync>
     */

    /* If we receive a sync without being in a room, drop the packet */
    if (session.gameroom.jid == NULL)
        return;

    char *data = wf_get_query_content(msg);

    if (data == NULL)
        return;

    gameroom_sync(data);

    free(data);
}

void gameroom_sync_init(void)
{
    gameroom_init(&session.gameroom.sync);
}

void gameroom_sync(const char *data)
{
    struct gameroom sync;

    gameroom_init(&sync);
    int ret = gameroom_parse(&sync, data);

    if (ret & GR_SYNC_SESSION)
    {
        if (sync.session.status == 2)
        {
            if (!session.gameroom.joined)
            {
                xmpp_send_iq_get(
                    JID_MS(session.online.channel),
                    NULL, NULL,
                    "<query xmlns='urn:cryonline:k01'>"
                    " <setcurrentclass current_class='%d'/>"
                    "</query>",
                    session.profile.curr_class);

                xmpp_send_iq_get(
                    JID_MS(session.online.channel),
                    xmpp_iq_session_join_cb, NULL,
                    "<query xmlns='urn:cryonline:k01'>"
                    " <session_join/>"
                    "</query>",
                    session.online.channel);
            }

            session.gameroom.joined = 1;

            if (sync.session.id != NULL
                && sync.session.id[0])
                xprintf("Session id: %s", sync.session.id);
        }
        else
        {
            if (session.gameroom.joined)
            {
                session.gameroom.joined = 0;
                session.gameroom.desired_status = GAMEROOM_READY;
            }
        }

        { /* Display new mission type/mode */
            const char *new_key =
                sync.mission.mission_key;
            const char *old_key =
                session.gameroom.sync.mission.mission_key;

            if ((new_key != NULL
                 && old_key != NULL
                 && 0 != strcmp(new_key, old_key))
                || (new_key != NULL && old_key == NULL))
            {
                xprintf("%s: %s %s (%s)",
                        LANG(update_mission),
                        sync.mission.type,
                        sync.mission.setting,
                        sync.mission.mode);
            }
        }
    }

    if (ret & GR_SYNC_AUTO_START)
    {
        /* Display auto-start state */

        if (session.gameroom.sync.auto_start.has_timeout !=
            sync.auto_start.has_timeout )
        {
            if (sync.auto_start.has_timeout)

            {
                char *s = LANG_FMT(update_auto_start,
                                   sync.auto_start.timeout_left);
                xprintf("%s", s);
                free(s);
            }
            else
            {
                xprintf("%s", LANG(update_auto_start_canceled));
            }
        }
    }

    if (ret & GR_SYNC_ROOM_MASTER)
    {
        /* Display new room master */

        const char *master = sync.room_master.master;
        const char *old_master = session.gameroom.sync.room_master.master;

        if (master != NULL
            && (old_master == NULL || 0 != strcmp(master, old_master)))
        {
            /* Get master player node */
            struct gr_core_player *p =
                list_get(session.gameroom.sync.core.players, master);

            if (p != NULL)
            {
                xprintf("%s: %s",
                        LANG(update_room_master),
                        p->nickname);
            }
        }
    }

    if (ret & GR_SYNC_CORE)
    {
        /* Get our player node */
        struct gr_core_player *p =
            list_get(sync.core.players,
                     session.profile.id);

        if (p != NULL)
        {
            session.gameroom.curr_team = p->team_id;
            session.gameroom.curr_status = p->status;
            session.profile.curr_class = p->class_id;
        }

        /* Auto-ready / unready */
        if (sync.auto_start.base.revision == 0
            && !session.gameroom.leaving
            && !cvar.wb_safemaster
            && session.gameroom.curr_status
               != session.gameroom.desired_status)
        {
            /* Check if status is not "denied" to prevent status update loop */
            if (session.gameroom.curr_status != GAMEROOM_RESTRICTED)
            {
                xmpp_iq_gameroom_setplayer(session.gameroom.curr_team,
                                           session.gameroom.desired_status,
                                           session.profile.curr_class,
                                           NULL, NULL);
            }
        }

        { /* Display total new player count */
            int new_count = sync.core.players->length;
            int old_count = session.gameroom.sync.core.players->length;

            if (new_count != old_count && new_count != 0)
            {
                xprintf("%s: %d/%d",
                        LANG(update_players),
                        new_count,
                        sync.custom_params.max_players);
            }
        }
    }

    gameroom_update(&session.gameroom.sync, &sync, ret);
}

void gameroom_sync_free(void)
{
    gameroom_free(&session.gameroom.sync);
}

void xmpp_iq_gameroom_sync_r(void)
{
    qh_register("gameroom_sync", 1, xmpp_iq_gameroom_sync_cb, NULL);
}
