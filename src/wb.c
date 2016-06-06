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


#include <wb_log.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#include <readline/readline.h>
#include <readline/history.h>

#include <wb_cvar.h>
#include <wb_option.h>
#include <wb_stream.h>
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>
#include <wb_tools.h>
#include <wb_session.h>
#include <wb_cmd.h>
#include <wb_dbus.h>

/** THEADS **/

#ifdef __MINGW32__
# include <windows.h>
# define sleep(x) Sleep(x)
#endif

void idle_close(const char *name);

void sigint_handler(int signum)
{
    session.active = 0;

#ifdef DBUS_API
    dbus_api_quit(0);
#endif

    pthread_exit(NULL);
}

void register_sigint_handler(void)
{
    signal(SIGINT, sigint_handler);
}

static int cmd_1arg(char *cmdline, char **arg1)
{
    char *saveptr = NULL;

    if (cmdline == NULL)
        return 0;

    *arg1 = strtok_r(cmdline, "", &saveptr);

    return *arg1 != NULL;
}

static int cmd_2args(char *cmdline, char **arg1, char **arg2)
{
    char *saveptr = NULL;

    if (cmdline == NULL)
        return 0;

    *arg1 = strtok_r(cmdline, " ", &saveptr);
    *arg2 = strtok_r(NULL, "", &saveptr);

    return *arg2 != NULL;
}

#if 0
static int cmd_3args(char *cmdline, char **arg1, char **arg2, char **arg3)
{
    char *saveptr = NULL;

    if (cmdline == NULL)
        return 0;

    *arg1 = strtok_r(cmdline, " ", &saveptr);
    *arg2 = strtok_r(NULL, " ", &saveptr);
    *arg3 = strtok_r(NULL, "", &saveptr);

    return *arg3 != NULL;
}
#endif

void *thread_readline(void *varg)
{
    int wfs = session.wfs;

    register_sigint_handler();
    using_history();

    do {
        char *buff_readline = readline("CMD# ");

        if (buff_readline == NULL)
        {
            if (session.active)
            {
                xmpp_iq_player_status(STATUS_OFFLINE);
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

            if (buff_readline[0] != '<')
            {
                char *cmd;
                char *args;

                cmd_2args(buff_readline, &cmd, &args);

                if (cvar_set(cmd, args))
                {
                    /* CVAR */
                }
                else if (0 == strcmp(cmd, "remove"))
                {
                    char *nickname;

                    if (cmd_1arg(args, &nickname))
                        cmd_remove_friend(nickname);
                }

                else if (0 == strcmp(cmd, "add"))
                {
                    char *nickname;

                    if (cmd_1arg(args, &nickname))
                        cmd_add_friend(nickname);
                }

                else if (0 == strcmp(cmd, "sleep"))
                {
                    char *delay;

                    if (cmd_1arg(args, &delay))
                        cmd_sleep(strtol(delay, NULL, 10));
                    else
                        cmd_sleep(1);
                }

                else if (0 == strcmp(cmd, "channel"))
                {
                    char *channel;

                    if (cmd_1arg(args, &channel))
                        cmd_channel(channel);
                }

                else if (0 == strcmp(cmd, "whisper"))
                {
                    char *nickname;
                    char *message;

                    if (cmd_2args(args, &nickname, &message))
                        cmd_whisper(nickname, message);
                }

                else if (0 == strcmp(cmd, "whois"))
                {
                    char *nickname;

                    if (cmd_1arg(args, &nickname))
                        cmd_whois(nickname, cmd_whois_console_cb, NULL);
                }

                else if (0 == strcmp(cmd, "missions"))
                {
                    cmd_missions(cmd_missions_console_cb, NULL);
                }

                else if (0 == strcmp(cmd, "say"))
                {
                    char *message;

                    if (cmd_1arg(args, &message))
                        cmd_say(message);
                }

                else if (0 == strcmp(cmd, "open"))
                {
                    char *mission;

                    if (cmd_1arg(args, &mission))
                        cmd_open(mission);
                    else
                        cmd_open(NULL);
                }

                else if (0 == strcmp(cmd, "name"))
                {
                    char *name;

                    if (cmd_1arg(args, &name))
                        cmd_name(name);
                }

                else if (0 == strcmp(cmd, "change"))
                {
                    char *mission;

                    if (cmd_1arg(args, &mission))
                        cmd_change(mission);
                    else
                        cmd_change(NULL);
                }

                else if (0 == strcmp(cmd, "ready"))
                {
                    char *class;

                    if (cmd_1arg(args, &class))
                        cmd_ready(class);
                    else
                        cmd_ready(NULL);
                }

                else if (0 == strcmp(cmd, "invite"))
                {
                    char *nickname;

                    if (cmd_1arg(args, &nickname))
                        cmd_invite(nickname, 0);
                }

                else if (0 == strcmp(cmd, "friends"))
                {
                    cmd_friends();
                }

                else if (0 == strcmp(cmd, "follow"))
                {
                    char *nickname;

                    if (cmd_1arg(args, &nickname))
                        cmd_follow(nickname);
                }

                else if (0 == strcmp(cmd, "master"))
                {
                    char *nickname;

                    if (cmd_1arg(args, &nickname))
                        cmd_master(nickname);
                }

                else if (0 == strcmp(cmd, "start"))
                {
                    cmd_start();
                }

                else if (0 == strcmp(cmd, "stats"))
                {
                    cmd_stats(cmd_stats_console_cb, NULL);
                }

                else if (0 == strcmp(cmd, "switch"))
                {
                    cmd_switch();
                }

                else if (0 == strcmp(cmd, "leave"))
                {
                    cmd_leave();
                }

                else if (0 == strcmp(cmd, "safe"))
                {
                    char *mission_name;

                    if (cmd_1arg(args, &mission_name))
                        cmd_safe(mission_name);
                    else
                        cmd_safe("tdm_airbase");
                }

                else
                    xprintf("Command not found: %s\n", cmd);
            }
            else
                send_stream(wfs, buff_readline, buff_size);
        }

        free(buff_readline);
    } while (session.active);

    idle_close("readline");
    pthread_exit(NULL);
}

#ifdef STAT_BOT

static void print_number_of_players_cb(const char *msg,
                                       enum xmpp_msg_type type,
                                       void *args)
{
    FILE *sfile = (FILE *) args;

    unsigned int count_all = 0;
    unsigned int count_pvp = 0;
    unsigned int count_pve = 0;

    const char *m = msg;
    while ((m = strstr(m, "<server ")))
    {
        /* Extract room jid */
        char *rjid = get_info(m, "resource='", "'", NULL);

        if (rjid != NULL)
        {
            unsigned int count = get_info_int(m, "online='", "'", NULL);

            if (strstr(rjid, "pve"))
                count_pve += count;
            else if (strstr(rjid, "pvp"))
                count_pvp += count;
            count_all += count;
        }

        free(rjid);
        ++m;
    }

    fprintf(sfile, "%u,%u,%u,%u\n",
            (unsigned) time(NULL), count_all, count_pve, count_pvp);

    fflush(sfile);
}

void *thread_stats(void *varg)
{
    int wfs = session.wfs;
    FILE *sfile = stdout;

    /* Dirty hack to wait for session initialisation */
    sleep(3);

    {
        char *s;

        FORMAT(s, "stats-%s-%ld.csv",
               cvar.game_server_name,
               time(NULL));

        sfile = fopen(s, "w");

        if (sfile == NULL)
        {
            eprintf("Unable to open %s for writting\n", s);
            sfile = stdout;
        }

        free(s);
    }

    fprintf(sfile, "Time,All players,PvE players,PvP players\n");

    register_sigint_handler();

    idh_register((t_uid *) "stats_num", 1, &print_number_of_players_cb, sfile);

    do {
        send_stream_ascii(wfs,
                          "<iq to='k01.warface' type='get' id='stats_num'>"
                          "<query xmlns='urn:cryonline:k01'>"
                          "<get_master_servers/>"
                          "</query>"
                          "</iq>");
        flush_stream(wfs);
        sleep(5);
    } while (session.active);


    if (sfile != stdout)
    {
        fclose(sfile);
    }

    xprintf("Closed stats\n");
    pthread_exit(NULL);
}
#endif

void *thread_dispatch(void *vargs)
{
    register_sigint_handler();

    XMPP_REGISTER_QUERY_HDLR();
    XMPP_WF_REGISTER_QUERY_HDLR();

    do {
        char *msg = read_stream(session.wfs);

        if (msg == NULL || strlen(msg) <= 0)
        {
            if (session.active)
            {
                xmpp_iq_player_status(STATUS_OFFLINE);
            }

            break;
        }

        { /* Replace any " with ' */
            for (char *s = msg; *s; ++s)
                if (*s == '"')
                    *s = '\'';
        }

        char *msg_id = get_msg_id(msg);
        enum xmpp_msg_type type = get_msg_type(msg);

        /* If we expect an answer from that ID */
        if (msg_id != NULL && idh_handle(msg_id, msg, type))
        {
            /* Good, we handled it */
        }
        /* If someone thinks we expected an answer */
        else if (type & (XMPP_TYPE_ERROR | XMPP_TYPE_RESULT))
        {
#ifdef DEBUG
            if (msg_id != NULL)
            {
                /* Unhandled stanza */
                eprintf("FIXME - Unhandled id: %s\n%s\n", msg_id, msg);
            }
#endif
        }
        /* If it wasn't handled and it's not a result */
        else
        {
            char *stanza = get_query_tag_name(msg);

            if (stanza == NULL)
            {
#ifdef DEBUG
                eprintf("FIXME - Unhandled msg:\n%s\n", msg);
#endif
            }
            /* Look if tagname is registered */
            else if (qh_handle(stanza, msg_id, msg))
            {
                /* Good, we handled it */
            }
            else
            {
#ifdef DEBUG
                /* Unhandled stanza */
                eprintf("FIXME - Unhandled query: %s\n%s\n", stanza, msg);
#endif
            }

            free(stanza);
        }

        free(msg);
        free(msg_id);

        session.xmpp.last_query = time(NULL);

        if (session.online.last_status_update + 5 * 60 < time(NULL))
        {
            xmpp_iq_player_status(session.online.status);
        }

    } while (session.active);

    idle_close("dispatch");
    pthread_exit(NULL);
}

void *thread_ping(void *vargs)
{
    int previous_ping = 0;
    const int ping_delay = 1 * 60;

    register_sigint_handler();

    do {

        if (session.xmpp.last_query + 4 * ping_delay < time(NULL))
        {
            xprintf("it's over.\n\n");
            break;
        }
        else if (session.xmpp.last_query + 3 * ping_delay < time(NULL))
        {
            xprintf("Stalling life... ");
            xmpp_iq_ping();
            previous_ping = 1;
        }
        else if (previous_ping)
        {
            xprintf("still there!\n");
            previous_ping = 0;
        }

        sleep(ping_delay);

    } while (session.active);

    idle_close("ping");
    pthread_exit(NULL);
}

static pthread_t th_dispatch;
static pthread_t th_readline;
static pthread_t th_ping;

void idle_init(void)
{
    if (pthread_create(&th_dispatch, NULL, &thread_dispatch, NULL) == -1)
        perror("pthread_create");
    else
        pthread_detach(th_dispatch);

#if ! defined DBUS_API || defined DEBUG
    if (pthread_create(&th_readline, NULL, &thread_readline, NULL) == -1)
        perror("pthread_create");
    else
        pthread_detach(th_readline);
#endif

    if (pthread_create(&th_ping, NULL, &thread_ping, NULL) == -1)
        perror("pthread_create");
    else
        pthread_detach(th_ping);

#ifdef STAT_BOT
    {
        pthread_t thread_dl;

        if (pthread_create(&thread_dl, NULL, &thread_stats, NULL) == -1)
            perror("pthread_create");

        pthread_detach(thread_dl);
    }

#endif
}

void idle_run(void)
{
#ifdef DBUS_API
    dbus_api_enter();
#else
    while (session.active)
        sleep(1);
#endif

    xprintf("Closed idle\n");
}

void idle_close(const char *name)
{
    xprintf("Closed %s\n", name);

#ifdef DBUS_API
    dbus_api_quit(0);
#endif

    session.active = 0;

    pthread_kill(th_ping, SIGINT);
    pthread_kill(th_dispatch, SIGINT);
    pthread_kill(th_readline, SIGINT);
}

int main(int argc, char *argv[])
{
    cvar_init();

    cvar_parse_file("wb.cfg");

    char *token = NULL;
    char *online_id = NULL;

    option_parse(argc, argv, &token, &online_id);

    /* Start of -- Legal Notices */

    if (isatty(STDOUT_FILENO))
    {
        xprintf(
            "\nWarfacebot Copyright (C) 2015, 2016 Levak Borok\n"
            "This program comes with ABSOLUTELY NO WARRANTY.\n"
            "This is free software, and you are welcome to redistribute it\n"
            "under certain conditions; see AGPLv3 Terms for details.\n\n");
    }

    /* End of -- Legal Notices */

    int wfs = connect_wf(cvar.online_server,
                         cvar.online_server_port);

    if (wfs > 0)
    {
        session_init(wfs);

        idle_init();

        xmpp_connect(token, online_id);

        idle_run();

        xmpp_close();

        session_free();
    }

    free(token);
    free(online_id);

    cvar_free();

    xprintf("Warface Bot closed!\n");

    return 0;
}
