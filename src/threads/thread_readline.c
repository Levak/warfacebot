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
#include <wb_threads.h>
#include <wb_xmpp_wf.h>
#include <wb_cmd.h>
#include <wb_cvar.h>
#include <wb_log.h>

#include <readline/readline.h>
#include <readline/history.h>

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

void thread_readline_init(void)
{
    /* Nothing to do */
}

void *thread_readline(void *varg)
{
    thread_register_sigint_handler();
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

        if (buff_readline[0] != '\0' && buff_readline[1] != '\0')
        {
            add_history(buff_readline);

            if (buff_readline[0] != '<')
            {
                char *cmd;
                char *args;

                cmd_2args(buff_readline, &cmd, &args);

                if (cmd == NULL)
                {
                    free(buff_readline);
                    continue;
                }

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

                else if (0 == strcmp(cmd, "stay"))
                {
                    char *duration = NULL;
                    char *unit;

                    if (cmd_2args(args, &duration, &unit))
                        cmd_stay(strtoll(duration, NULL, 10), unit);
                    else if (duration != NULL)
                        cmd_stay(strtoll(duration, NULL, 10), NULL);
                    else
                        cmd_stay(1, "hour");
                }

                else if (0 == strcmp(cmd, "sponsor"))
                {
                    char *sponsor;

                    if (cmd_1arg(args, &sponsor))
                        cmd_sponsor(sponsor);
                }

                else if (0 == strcmp(cmd, "switch"))
                {
                    cmd_switch();
                }

                else if (0 == strcmp(cmd, "last"))
                {
                    char *nickname;

                    if (cmd_1arg(args, &nickname))
                        cmd_last(nickname);
                }

                else if (0 == strcmp(cmd, "unready"))
                {
                    cmd_unready();
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
                thread_sendstream_post_new_msg(strdup(buff_readline));
        }

        free(buff_readline);
    } while (session.active);


    return thread_close();
}
