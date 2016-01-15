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

#include <wb_session.h>
#include <wb_xmpp_wf.h>
#include <wb_tools.h>
#include <wb_pvp_maps.h>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>

#ifdef __MINGW32__
# include <Winsock.h>
#else
# include <sys/socket.h>
# include <netinet/in.h>
# include <netdb.h>
#endif

struct cb_args
{
    char *nickname;
    char *online_id;
    unsigned int profile_id;
};

static const char *blacklist_service = NULL; /* TODO */
static int is_blacklist(const char *nickname)
{
    if (blacklist_service == NULL)
        return 0;

    int fd = -1;
    {
        fd = socket(AF_INET, SOCK_STREAM, 0);

        struct sockaddr_in serv_addr;
        struct hostent *server;

        server = gethostbyname(blacklist_service);
        if (server == NULL)
        {
            fprintf(stderr, "ERROR gethostbyname %s\n", strerror(errno));
            return 0;
        }

        memset((char *) &serv_addr, 0, sizeof(serv_addr));
        memcpy((char *) &serv_addr.sin_addr.s_addr,
               (char *) server->h_addr,
               server->h_length);

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(80);

        if (connect(fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        {
            fprintf(stderr, "ERROR connect %s\n", strerror(errno));
            return 0;
        }
    }

    int exists = 0;
    {
        char *request;
        FORMAT(request,
               "GET /blacklist.php?name=%s HTTP/1.1\n"
               "Host: %s\n\n",
               nickname, blacklist_service);

        send(fd, request, strlen(request), 0);
        free(request);

        char buff[4096];
        memset(buff, 0, sizeof(buff));
        recv(fd, buff, sizeof(buff), 0);

        char *header = get_info(buff, "HTTP", "\n", NULL);

        if (header != NULL)
            exists = strstr(header, "200 OK") != NULL;

        free(header);
    }

    return exists;
}

static void xmpp_iq_gameroom_kick_cb(const char *msg, void *args)
{
    struct cb_args *a = (struct cb_args *) args;

    if (xmpp_is_error(msg))
        printf("Error while kicking %s\n", a->nickname);

    free(a->online_id);
    free(a->nickname);
    free(a);
}

static void xmpp_iq_ppi_cb(const char *info, void *args)
{
    struct cb_args *a = (struct cb_args *) args;

    if (info == NULL)
    {
        free(a->online_id);
        free(a->nickname);
        free(a);

        return;
    }

    unsigned pvp_kills = get_info_int(info, "pvp_kills='", "'", NULL);
    unsigned pvp_deaths = get_info_int(info, "pvp_deaths='", "'", NULL);

    float ratio = (pvp_deaths == 0)
        ? 0.0f
        : ((float) pvp_kills) / ((float) pvp_deaths);

    if (ratio > 3.0f)
    {
        printf("Kicked high KDR (%f) %s\n", ratio, a->nickname);
        xmpp_iq_gameroom_kick(a->profile_id,
                              xmpp_iq_gameroom_kick_cb, a);
    }
    else
    {
        free(a->online_id);
        free(a->nickname);
        free(a);
    }
}

static void *thread_checknkick(void *vargs)
{
    struct cb_args *a = (struct cb_args *) vargs;

    if (is_blacklist(a->nickname))
    {
        printf("Kicked blacklisted %s\n", a->nickname);
        xmpp_iq_gameroom_kick(a->profile_id,
                              xmpp_iq_gameroom_kick_cb, a);
    }
    else
    {
        xmpp_iq_peer_player_info(a->online_id, xmpp_iq_ppi_cb, a);
    }

    pthread_exit(NULL);
}


static void check_and_kick(struct cb_args *a)
{
    pthread_t thread_ck;

    if (pthread_create(&thread_ck, NULL, thread_checknkick, a) == -1)
        perror("pthread_create");

    pthread_detach(thread_ck);
}


static void xmpp_iq_pigs_cb(const char *info,
                            void *args)
{
    if (info == NULL)
        return;

    unsigned rank = get_info_int(info, "rank='", "'", NULL);

    struct cb_args *a = calloc(1, sizeof (struct cb_args));

    a->profile_id = get_info_int(info, "profile_id='", "'", NULL);
    a->online_id = get_info(info, "online_id='", "'", NULL);
    a->nickname = get_info(info, "nickname='", "'", NULL);

    if (rank < 15)
    {
        printf("Kicked low level %s\n", a->nickname);
        xmpp_iq_gameroom_kick(a->profile_id,
                              xmpp_iq_gameroom_kick_cb, a);
    }
    else
    {
        check_and_kick(a);
    }
}

static unsigned int player_count = 0;
static void xmpp_iq_presence_cb(const char *msg_id,
                                const char *msg,
                                void *args)
{
    /* Wtf are we doing here if gameroom has closed? */
    if (msg == NULL
        || session.safemaster == 0
        || session.gameroom_jid == NULL)
    {
        player_count = 0;
        return;
    }

    /* If we've received a presence that is not from the gameroom,
       register a new handler */
    if (strstr(msg, session.gameroom_jid) == NULL)
    {
        qh_register("presence", 0, xmpp_iq_presence_cb, NULL);
        return;
    }

    char *nickname = get_info(msg, "warface/", "'", NULL);

    /* Someone is leaving */
    if (strstr(msg, "unavailable"))
    {
        player_count--;

        /* We are not leaving, so register a new handler */
        if (strcmp(session.nickname, nickname) != 0)
            qh_register("presence", 0, xmpp_iq_presence_cb, NULL);
    }

    /* Someone is joining */
    else
    {
        xmpp_iq_profile_info_get_status(nickname, xmpp_iq_pigs_cb, NULL);

        player_count++;

        if (player_count >= 8)
            xmpp_iq_gameroom_askserver(NULL, NULL);

        qh_register("presence", 0, xmpp_iq_presence_cb, NULL);
    }

    printf("%d players in the room\n", player_count);

    free(nickname);
}

static void xmpp_iq_open_room_cb(const char *msg,
                                 void *args)
{
    char *mission_key = (char *) args;

    session.safemaster = 1;

    player_count = 0;

    xmpp_iq_gameroom_setname("FairGame (rank > 15 & kdr < 3)", NULL, NULL);
    xmpp_iq_gameroom_update_pvp(mission_key,
                                PVP_AUTOBALANCE | PVP_DEADCHAT,
                                16, 0, NULL, NULL);

    qh_register("presence", 0, xmpp_iq_presence_cb, NULL);

    free(mission_key);
}

static void xmpp_iq_join_channel_cb(void *args)
{
    char *mission_key = (char *) args;

    xmpp_iq_gameroom_open(mission_key, ROOM_PVP,
                          xmpp_iq_open_room_cb, args);
}

void cmd_safe(const char *mission_name)
{
    if (mission_name == NULL)
        mission_name = "tdm_airbase";

    if (strlen(mission_name) != 36) /* not an uuid */
    {
        struct mission *m = mission_list_get(mission_name);

        if (m != NULL)
        {
            void *args = strdup(m->mission_key);

            xmpp_iq_join_channel("pvp_pro_5",
                                 xmpp_iq_join_channel_cb, args);
        }
    }
    else
    {
        void *args = strdup(mission_name);

        xmpp_iq_join_channel("pvp_pro_5",
                             xmpp_iq_join_channel_cb, args);
    }
}
