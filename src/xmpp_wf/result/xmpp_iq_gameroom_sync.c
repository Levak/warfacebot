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

#include <wb_tools.h>
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>
#include <wb_list.h>
#include <wb_session.h>
#include <wb_cvar.h>
#include <wb_log.h>

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

            if (session.gameroom.leave_timeout > time(NULL)
                || !cvar.wb_leave_on_start
                || cvar.wb_safemaster)
            {
                xmpp_iq_gameroom_setplayer(session.gameroom.curr_team,
                                           GAMEROOM_UNREADY,
                                           session.profile.curr_class,
                                           NULL, NULL);

            }
            else
            {
                xmpp_iq_gameroom_leave();
                left = 1;
            }

            xprintf("Game room started! %s "
                    "(IP/PORT/S/RID/SID: %s %d %s %s %s)\n",
                    left ? "Leave..." : "But stay here.",
                    ip, port, server, room_id, session_id);
        }

        free(ip);
        free(server);
        free(room_id);
        free(session_id);
        free(data);
    }
}

typedef void (* f_sync_func)(void *local, const char *node);

static int _player_cmp(struct gr_core_player *p, char *profile_id)
{
    return strcmp(p->profile_id, profile_id);
}

static void _player_free(struct gr_core_player *p)
{
    free(p->nickname);
    free(p->clan_name);
    free(p->profile_id);
    free(p->online_id);
    free(p->group_id);
    free(p->region_id);

    free(p);
}

static inline int gameroom_sync_node(s_gr_sync *local,
                                     f_sync_func sync,
                                     const char *node)
{
    if (node == NULL)
        return 0;

    unsigned int revision = get_info_int(node, "revision='", "'", NULL);

    if (revision > local->revision)
    {
        local->revision = revision;
        sync(local, node);

        return local->type;
    }

    return 0;
}

#define SYNC_INT(Local, Node, Name)                      \
    (Local) = get_info_int(Node, Name "='", "'", NULL)

#define SYNC_FLT(Local, Node, Name)                      \
    (Local) = get_info_float(Node, Name "='", "'", NULL)

#define SYNC_STR(Local, Node, Name) do {                 \
        free(Local);                                     \
        (Local) = get_info(Node, Name "='", "'", NULL);  \
    } while (0)                                          \

static void _sync_core(s_gr_core *local, const char *node)
{
    SYNC_INT(local->teams_switched, node, "teams_switched");
    SYNC_INT(local->private, node, "private");
    SYNC_INT(local->can_start, node, "can_start");
    SYNC_INT(local->team_balanced, node, "team_balanced");
    SYNC_INT(local->min_ready_players, node, "min_ready_players");

    SYNC_STR(local->room_name, node, "room_name");

    char *players = get_info(node, "<players>", "</players>", NULL);

    /* Loop foreach players */
    {
        const char *m = players;

        list_empty(session.gameroom.sync.core.players);

        while ((m = strstr(m, "<player")))
        {
            char *player = get_info(m, "<player", "/>", NULL);

            struct gr_core_player *p =
                calloc(1, sizeof(struct gr_core_player));

            SYNC_STR(p->nickname, player, "nickname");
            SYNC_STR(p->clan_name, player, "clanName");
            SYNC_STR(p->profile_id, player, "profile_id");
            SYNC_STR(p->online_id, player, "online_id");
            SYNC_STR(p->group_id, player, "group_id");
            SYNC_STR(p->region_id, player, "region_id");

            SYNC_INT(p->class_id, player, "class_id");
            SYNC_INT(p->team_id, player, "team_id");
            SYNC_INT(p->status, player, "status");
            SYNC_INT(p->presence, player, "presence");
            SYNC_INT(p->observer, player, "observer");
            SYNC_INT(p->experience, player, "experience");
            SYNC_INT(p->rank, player, "rank");
            SYNC_INT(p->banner.badge, player, "banner_badge");
            SYNC_INT(p->banner.mark, player, "banner_mark");
            SYNC_INT(p->banner.stripe, player, "banner_stripe");

            SYNC_FLT(p->skill, player, "skill");

            list_add(session.gameroom.sync.core.players, p);

            free(player);

            ++m;
        }
    }

    free(players);
}

static void _sync_custom_params(s_gr_custom_params *local, const char *node)
{
    SYNC_INT(local->friendly_fire, node, "friendly_fire");
    SYNC_INT(local->enemy_outlines, node, "enemy_outlines");
    SYNC_INT(local->auto_team_balance, node, "auto_team_balance");
    SYNC_INT(local->dead_can_chat, node, "dead_can_chat");
    SYNC_INT(local->join_in_the_process, node, "join_in_the_process");

    SYNC_INT(local->max_players, node, "max_players");
    SYNC_INT(local->round_limit, node, "round_limit");
    SYNC_INT(local->class_restriction, node, "class_restriction");
    SYNC_INT(local->inventory_slot, node, "inventory_slot");
}

static void _sync_mission(s_gr_mission *local, const char *node)
{
    SYNC_STR(local->mission_key, node, "mission_key");
    SYNC_STR(local->name, node, "name");
    SYNC_STR(local->setting, node, "setting");
    SYNC_STR(local->mode, node, "mode");
    SYNC_STR(local->mode_name, node, "mode_name");
    SYNC_STR(local->mode_icon, node, "mode_icon");
    SYNC_STR(local->description, node, "description");
    SYNC_STR(local->image, node, "image");
    SYNC_STR(local->difficulty, node, "difficulty");
    SYNC_STR(local->type, node, "type");
    SYNC_STR(local->time_of_day, node, "time_of_day");

    SYNC_INT(local->no_teams, node, "no_teams");
}

static void _sync_session(s_gr_session *local, const char *node)
{
    SYNC_STR(local->id, node, "id");

    SYNC_INT(local->status, node, "status");
    SYNC_INT(local->game_progress, node, "game_progress");
    SYNC_INT(local->start_time, node, "start_time");
}

static void _sync_room_master(s_gr_room_master *local, const char *node)
{
    SYNC_STR(local->master, node, "master");
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
        </game_room>
       </gameroom_sync>
     */

    char *data = wf_get_query_content(msg);

    if (data == NULL)
        return;

    gameroom_sync(data);

    free(data);
}

void gameroom_sync_init(void)
{
    session.gameroom.sync.core.players =
        list_new((f_list_cmp) _player_cmp,
                 (f_list_free) _player_free);

    session.gameroom.sync.core.base.type = GR_SYNC_CORE;
    session.gameroom.sync.custom_params.base.type = GR_SYNC_CUSTOM_PARAMS;
    session.gameroom.sync.mission.base.type = GR_SYNC_MISSION;
    session.gameroom.sync.session.base.type = GR_SYNC_SESSION;
    session.gameroom.sync.room_master.base.type = GR_SYNC_ROOM_MASTER;
}

void gameroom_sync(const char *data)
{
    int ret = 0;

    char *core_node = get_info(data, "<core ", "</core>", NULL);
    char *session_node = get_info(data, "<session ", "/>", NULL);
    char *mission_node = get_info(data, "<mission ", ">", NULL);
    char *room_master_node = get_info(data, "<room_master ", "/>", NULL);
    char *custom_params_node = get_info(data, "<custom_params ", "/>", NULL);

    ret |= gameroom_sync_node((s_gr_sync *) &session.gameroom.sync.core,
                              (f_sync_func) _sync_core,
                              core_node);

    ret |= gameroom_sync_node((s_gr_sync *) &session.gameroom.sync.custom_params,
                              (f_sync_func) _sync_custom_params,
                              custom_params_node);

    ret |= gameroom_sync_node((s_gr_sync *) &session.gameroom.sync.mission,
                              (f_sync_func) _sync_mission,
                              mission_node);

    ret |= gameroom_sync_node((s_gr_sync *) &session.gameroom.sync.session,
                              (f_sync_func) _sync_session,
                              session_node);

    ret |= gameroom_sync_node((s_gr_sync *) &session.gameroom.sync.room_master,
                              (f_sync_func) _sync_room_master,
                              room_master_node);

    free(core_node);
    free(session_node);
    free(mission_node);
    free(room_master_node);
    free(custom_params_node);

    if (ret & GR_SYNC_SESSION)
    {
        if (session.gameroom.sync.session.status == 2)
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

            if (session.gameroom.sync.session.id != NULL
                && session.gameroom.sync.session.id[0])
                xprintf("Session id: %s\n", session.gameroom.sync.session.id);
        }
        else
        {
            if (session.gameroom.joined)
            {
                session.gameroom.joined = 0;
            }
        }
    }

    if (ret & GR_SYNC_CORE)
    {
        struct gr_core_player *p =
            list_get(session.gameroom.sync.core.players,
                     session.profile.id);

        if (p != NULL)
        {
            session.gameroom.curr_team = p->team_id;
            session.gameroom.status = p->status;
            session.profile.curr_class = p->class_id;
        }

        if (!session.gameroom.leaving
            && !cvar.wb_safemaster
            && session.gameroom.status == GAMEROOM_UNREADY)
        {
            xmpp_iq_gameroom_setplayer(session.gameroom.curr_team,
                                       GAMEROOM_READY,
                                       session.profile.curr_class,
                                       NULL, NULL);
        }
    }

    if (ret & GR_SYNC_MISSION)
    {
        /* Update cached infos */

        free(session.online.place_token);
        session.online.place_token = NULL;
        free(session.online.place_info_token);
        session.online.place_info_token = NULL;
        free(session.online.mode_info_token);
        session.online.mode_info_token = NULL;
        free(session.online.mission_info_token);
        session.online.mission_info_token = NULL;

        if (session.online.status & STATUS_LOBBY)
        {
            session.online.place_token =
                strdup("@ui_playerinfo_inlobby");
        }
        else if (session.gameroom.jid != NULL)
        {
            const char *mode = session.gameroom.sync.mission.mode;

            if (mode != NULL)
            {
                if (strcmp(mode, "pve") == 0)
                {
                    session.online.place_token =
                        strdup("@ui_playerinfo_pveroom");

                    if (session.gameroom.sync.mission.type != NULL)
                    {
                        session.online.place_info_token =
                            strdup(session.gameroom.sync.mission.type);
                    }
                }
                else
                {
                    session.online.place_token =
                        strdup("@ui_playerinfo_pvproom");
                    session.online.place_info_token =
                        strdup("@ui_playerinfo_location");

                    if (session.gameroom.sync.mission.mode_name != NULL)
                        session.online.mode_info_token =
                            strdup(session.gameroom.sync.mission.mode_name);
                    if (session.gameroom.sync.mission.name != NULL)
                        session.online.mission_info_token =
                            strdup(session.gameroom.sync.mission.name);
                }
            }
        }
    }
}

void gameroom_sync_free(void)
{
    if (session.gameroom.sync.core.players != NULL)
    {
        list_free(session.gameroom.sync.core.players);
        session.gameroom.sync.core.players = NULL;
    }

    free(session.gameroom.sync.core.room_name);
    session.gameroom.sync.core.room_name = NULL;

    free(session.gameroom.sync.mission.mission_key);
    session.gameroom.sync.mission.mission_key = NULL;
    free(session.gameroom.sync.mission.name);
    session.gameroom.sync.mission.name = NULL;
    free(session.gameroom.sync.mission.setting);
    session.gameroom.sync.mission.setting = NULL;
    free(session.gameroom.sync.mission.mode);
    session.gameroom.sync.mission.mode = NULL;
    free(session.gameroom.sync.mission.mode_name);
    session.gameroom.sync.mission.mode_name = NULL;
    free(session.gameroom.sync.mission.mode_icon);
    session.gameroom.sync.mission.mode_icon = NULL;
    free(session.gameroom.sync.mission.description);
    session.gameroom.sync.mission.description = NULL;
    free(session.gameroom.sync.mission.image);
    session.gameroom.sync.mission.image = NULL;
    free(session.gameroom.sync.mission.difficulty);
    session.gameroom.sync.mission.difficulty = NULL;
    free(session.gameroom.sync.mission.type);
    session.gameroom.sync.mission.type = NULL;
    free(session.gameroom.sync.mission.time_of_day);
    session.gameroom.sync.mission.time_of_day = NULL;

    free(session.gameroom.sync.session.id);
    session.gameroom.sync.session.id = NULL;

    free(session.gameroom.sync.room_master.master);
    session.gameroom.sync.room_master.master = NULL;

    session.gameroom.sync.core.base.revision = 0;
    session.gameroom.sync.custom_params.base.revision = 0;
    session.gameroom.sync.mission.base.revision = 0;
    session.gameroom.sync.session.base.revision = 0;
    session.gameroom.sync.room_master.base.revision = 0;
}

void xmpp_iq_gameroom_sync_r(void)
{
    qh_register("gameroom_sync", 1, xmpp_iq_gameroom_sync_cb, NULL);
}
