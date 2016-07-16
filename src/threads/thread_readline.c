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
#include <wb_list.h>
#include <wb_log.h>

#include <readline/readline.h>
#include <readline/history.h>

enum cmd_status
{
    CMD_SUCCESS,
    CMD_MISSING_ARGS,
};

typedef enum cmd_status (*f_cmd_parse)(char *line);

struct cmd
{
    const char *name;
    f_cmd_parse parse;
    const char *usage;
    const char *description;
};

static struct list *cmd_list = NULL;

static int cmd_cmp(struct cmd *cmd, const char *name)
{
    return strcmp(cmd->name, name);
}

static void cmd_free(struct cmd *cmd)
{
    free(cmd);
}

static int cmd_1arg(char *cmdline,
                    char **arg1)
{
    char *saveptr = NULL;

    if (cmdline == NULL)
        return 0;

    *arg1 = strtok_r(cmdline, "", &saveptr);

    if (*arg1 != NULL)
        *arg1 += strspn(*arg1, " ");

    return (*arg1 != NULL) ? 1 : 0;
}

static int cmd_2args(char *cmdline,
                     char **arg1,
                     char **arg2)
{
    char *saveptr = NULL;

    if (cmdline == NULL)
        return 0;

    *arg1 = strtok_r(cmdline, " ", &saveptr);
    *arg2 = strtok_r(NULL, "", &saveptr);

    if (*arg1 != NULL)
        *arg1 += strspn(*arg1, " ");

    if (*arg2 != NULL)
        *arg2 += strspn(*arg2, " ");

    return ((*arg1 != NULL) ? 1 : 0)
        + ((*arg2 != NULL) ? 1 : 0);
}

#if 0
static int cmd_3args(char *cmdline,
                     char **arg1,
                     char **arg2,
                     char **arg3)
{
    char *saveptr = NULL;

    if (cmdline == NULL)
        return 0;

    *arg1 = strtok_r(cmdline, " ", &saveptr);
    *arg2 = strtok_r(NULL, " ", &saveptr);
    *arg3 = strtok_r(NULL, "", &saveptr);

    if (*arg1 != NULL)
        *arg1 += strspn(*arg1, " ");

    if (*arg2 != NULL)
        *arg2 += strspn(*arg2, " ");

    if (*arg3 != NULL)
        *arg3 += strspn(*arg3, " ");

    return ((*arg1 != NULL) ? 1 : 0)
        + ((*arg2 != NULL) ? 1 : 0)
        + ((*arg3 != NULL) ? 1 : 0);
}
#endif

static void cmd_help_cb(const struct cmd *c, void *args)
{
    xprintf("%9s %-20s %s\n", c->name, c->usage, c->description);
}

static void cmd_help_wrapper(void)
{
    list_foreach(cmd_list,
                 (f_list_callback) cmd_help_cb,
                 NULL);
}

void parse_cmd(const char *line)
{
    char *command = strdup(line);

    if (command[0] != '<')
    {
        const char sep[] = " \t";
        char *saveptr;
        char *cmd = strtok_r(command, sep, &saveptr);
        char *args = strtok_r(NULL, "", &saveptr);

        if (cmd == NULL)
        {
            free(command);
            return;
        }

        if (args != NULL)
            args += strspn(args, sep);
        else
            args = "";

        if (cvar_set(cmd, args))
        {
            /* CVAR */
        }
        else
        {
            struct cmd *c = list_get(cmd_list, cmd);

            if (c != NULL)
            {
                enum cmd_status s = c->parse(args);

                switch (s)
                {
                    case CMD_MISSING_ARGS:
                        eprintf("Usage: %s %s\n",
                                c->name,
                                c->usage);
                        break;
                    default:
                        break;
                }
            }
            else
            {
                xprintf("Command not found: %s\n", cmd);
            }
        }

        free(command);
    }
    else
        thread_sendstream_post_new_msg(command);
}

/********** Cmd Parsers ***********/

#define X0(Name, _1, _2)                                        \
    static enum cmd_status cmd_ ## Name ## _parse(char *line) { \
        void cmd_ ## Name ## _wrapper(void);                    \
        cmd_ ## Name ## _wrapper();                             \
        return CMD_SUCCESS;                                     \
    }                                                           \

#define X1(Name, _1, Min)                                       \
    static enum cmd_status cmd_ ## Name ## _parse(char *line) { \
        enum cmd_status ret = CMD_SUCCESS;                      \
        char *a0 = NULL;                                        \
        int count = cmd_1arg(line, &a0);                        \
        void cmd_ ## Name ## _wrapper(const char *a0);          \
        if (count >= 1) {                                       \
            cmd_ ## Name ## _wrapper(a0);                       \
        } else if (count == 0 && Min == 0) {                    \
            cmd_ ## Name ## _wrapper(NULL);                     \
        } else {                                                \
            ret = CMD_MISSING_ARGS;                             \
        }                                                       \
        return ret;                                             \
    }                                                           \

#define X2(Name, _1, Min)                                       \
    static enum cmd_status cmd_ ## Name ## _parse(char *line) { \
        enum cmd_status ret = CMD_SUCCESS;                      \
        char *a0 = NULL, *a1 = NULL;                            \
        int count = cmd_2args(line, &a0, &a1);                  \
        void cmd_ ## Name ## _wrapper(const char *a0,           \
                                   const char *a1);             \
        if (count >= 2) {                                       \
            cmd_ ## Name ## _wrapper(a0, a1);                   \
        } else if (count == 1 && Min <= 1) {                    \
            cmd_ ## Name ## _wrapper(a0, NULL);                 \
        } else if (count == 0 && Min == 0) {                    \
            cmd_ ## Name ## _wrapper(NULL, NULL);               \
        } else {                                                \
            ret = CMD_MISSING_ARGS;                             \
        }                                                       \
        return ret;                                             \
    }                                                           \

#define X3(Name, _1, Min)                                       \
    static enum cmd_status cmd_ ## Name ## _parse(char *line) { \
        enum cmd_status ret = CMD_SUCCESS;                      \
        char *a0 = NULL, *a1 = NULL, *a2 = NULL;                \
        int count = cmd_3args(line, &a0, &a1, &a2);             \
        void cmd_ ## Name ## _wrapper(const char *a0,           \
                                   const char *a1,              \
                                   const char *a2);             \
        if (count >= 3) {                                       \
            cmd_ ## Name ## _wrapper(a0, a1, a2);               \
        } else if (count == 2 && Min <= 2) {                    \
            cmd_ ## Name ## _wrapper(a0, a1, NULL);             \
        } else if (count == 1 && Min <= 1) {                    \
            cmd_ ## Name ## _wrapper(a0, NULL, NULL);           \
        } else if (count == 0 && Min == 0) {                    \
            cmd_ ## Name ## _wrapper(NULL, NULL, NULL);         \
        } else {                                                \
            ret = CMD_MISSING_ARGS;                             \
        }                                                       \
        return ret;                                             \
    }                                                           \

#define XARG0()           0
#define XARG1(Arg1)       1
#define XARGN(Arg1)       1
#define XARGS(Arg1, Arg2) 1 + Arg2
#define XOPT(Arg1)        0

    CMD_LIST_CONSOLE

#undef X0
#undef X1
#undef X2
#undef X3
#undef XARG0
#undef XARG1
#undef XARGN
#undef XARGS
#undef XOPT

/********* Init cmd list **********/

void thread_readline_init(void)
{
    cmd_list = list_new((f_list_cmp) cmd_cmp,
                        (f_list_free) cmd_free);

#define X0(Name, Desc, Usage) X(Name, Desc, Usage)
#define X1(Name, Desc, Usage) X(Name, Desc, Usage)
#define X2(Name, Desc, Usage) X(Name, Desc, Usage)
#define X3(Name, Desc, Usage) X(Name, Desc, Usage)
#define X(Name, Desc, Usage) do {                                 \
        struct cmd *c = calloc(1, sizeof (struct cmd));           \
        c->name = #Name;                                          \
        c->parse = cmd_ ## Name ## _parse;                        \
        c->usage = Usage;                                         \
        c->description = Desc;                                    \
        list_add(cmd_list, c);                                    \
    } while (0)

#define XARG0()           ""
#define XARG1(Arg1)       "<" #Arg1 ">"
#define XARGN(Arg1)       "<" #Arg1 "...>"
#define XARGS(Arg1, Arg2) "<" #Arg1 "> " Arg2
#define XOPT(Arg1)        "[" Arg1 "]"

    CMD_LIST_CONSOLE

#undef X
#undef X0
#undef X1
#undef X2
#undef X3
#undef XARG0
#undef XARG1
#undef XARGN
#undef XARGS
#undef XOPT
}

/********* Readline *********/

void *thread_readline(void *vargs)
{
    struct thread *t = (struct thread *) vargs;

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

            parse_cmd(buff_readline);
        }

        free(buff_readline);
    } while (session.active);

    list_free(cmd_list);
    cmd_list = NULL;

    return thread_close(t);
}
