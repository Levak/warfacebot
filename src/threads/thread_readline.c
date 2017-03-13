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

#include <wb_session.h>
#include <wb_threads.h>
#include <wb_xmpp_wf.h>
#include <wb_cmd.h>
#include <wb_cvar.h>
#include <wb_list.h>
#include <wb_log.h>
#include <wb_dbus.h>
#include <wb_lang.h>

#include <readline/readline.h>
#include <readline/history.h>

#include <setjmp.h>
#include <signal.h>

enum cmd_status
{
    CMD_SUCCESS,
    CMD_MISSING_ARGS,
    CMD_CVAR_ERROR,
};

struct cmd;

typedef enum cmd_status (*f_cmd_parse)(struct cmd *c, char *line);
typedef int (*f_cmd_complete)(struct list *l, int arg_index);

struct cmd
{
    const char *name;
    f_cmd_parse parse;
    f_cmd_complete complete;
    const char *usage;
    const char *description;
};

static struct list *cmd_list = NULL;

static int cmd_match(const struct cmd *cmd, const char *text, size_t len)
{
    return strncmp(cmd->name, text, len);
}

static char *cmd_copy(const struct cmd *cmd)
{
    return strdup(cmd->name);
}

static int cmd_cmp(const struct cmd *cmd, const char *name)
{
    return strcmp(cmd->name, name);
}

static void cmd_free(struct cmd *cmd)
{
    free(cmd);
}

/**
 * Special parsing method for CVARs.
 * This method skips the next couple spaces and equal signs if any.
 */
static enum cmd_status cvar_parse(struct cmd *c, char *line)
{
    if (line != NULL)
    {
        line += strspn(line, " \t=");
    }

    if (cvar_set(c->name, line))
        return CMD_SUCCESS;
    else
        return CMD_CVAR_ERROR;
}

/**
 * Special completion routinue for CVARs.
 * There are no completions for CVAR arguments.
 */
static int cvar_complete(struct list *l, int arg_index)
{
    return 1;
}

/**
 * Trim with \0 the ending spaces of a string.
 */
static void trim_end(char *s)
{
    char *end = s + strlen(s) - 1;

    for (; end[0] && isspace(end[0]); end[0] = 0, --end)
        continue;
}

static int cmd_1arg(char *cmdline,
                    char **arg1)
{
    char *saveptr = NULL;

    if (cmdline == NULL)
        return 0;

    *arg1 = strtok_r(cmdline, "", &saveptr);

    if (*arg1 != NULL)
    {
        *arg1 += strspn(*arg1, " ");
        trim_end(*arg1);
    }

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
    {
        *arg2 += strspn(*arg2, " ");
        trim_end(*arg2);
    }

    return ((*arg1 != NULL) ? 1 : 0)
        + ((*arg2 != NULL) ? 1 : 0);
}

#ifdef CMD_3_ARGS
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
    {
        *arg3 += strspn(*arg3, " ");
        trim_end(*arg3);
    }

    return ((*arg1 != NULL) ? 1 : 0)
        + ((*arg2 != NULL) ? 1 : 0)
        + ((*arg3 != NULL) ? 1 : 0);
}
#endif

/**
 * Print usage and description for a given Command (excluding CVARs)
 */
static void cmd_help_cb(const struct cmd *c, void *args)
{
    if (c->usage != NULL && c->description != NULL)
        xprintf("%15s %-20s %s", c->name, c->usage, c->description);
}

/**
 * Help command entry point.
 *
 * This command is defined internally in order to keep cmd_list internal to
 * the readline thread.
 */
static void cmd_help_wrapper(void)
{
    list_foreach(cmd_list,
                 (f_list_callback) cmd_help_cb,
                 NULL);
}

/**
 * Parse and execute a given command line.
 */
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

        struct cmd *c = list_get(cmd_list, cmd);

        if (c != NULL)
        {
            enum cmd_status s = c->parse(c, args);

            switch (s)
            {
                case CMD_MISSING_ARGS:
                    eprintf("%s: %s %s",
                            LANG(error_command_usage),
                            c->name,
                            c->usage);
                    break;
                case CMD_CVAR_ERROR:
                    eprintf("%s: `%s`",
                            LANG(error_set_cvar),
                            c->name);
                    break;
                default:
                    break;
            }
        }
        else
        {
            xprintf("%s: %s", LANG(error_command_not_found), cmd);
        }

        free(command);
    }
    else
        thread_sendstream_post_new_msg(command);
}

/********** Cmd Parsers ***********/

#define X0(Name, _1, _2)                                        \
    static enum cmd_status cmd_ ## Name ## _parse(struct cmd *c,\
                                                  char *line) { \
        void cmd_ ## Name ## _wrapper(void);                    \
        cmd_ ## Name ## _wrapper();                             \
        return CMD_SUCCESS;                                     \
    }                                                           \

#define X1(Name, _1, Min)                                       \
    static enum cmd_status cmd_ ## Name ## _parse(struct cmd *c,\
                                                  char *line) { \
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
    static enum cmd_status cmd_ ## Name ## _parse(struct cmd *c,\
                                                  char *line) { \
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
    static enum cmd_status cmd_ ## Name ## _parse(struct cmd *c,\
                                                  char *line) { \
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

/********** Cmd Completions ***********/

#define X0(Name, _1, _2)                                                \
    static int cmd_ ## Name ## _complete(struct list *l, int arg_i) {   \
        return 1;                                                       \
    }                                                                   \

#define X1(Name, _1, _2)                                                \
    static int cmd_ ## Name ## _complete(struct list *l, int i) {       \
        if (i > 1)                                                      \
            return 1;                                                   \
        int cmd_ ## Name ## _completions(struct list *l);               \
        return cmd_ ## Name ## _completions(l);                         \
    }                                                                   \

#define X2(Name, _1, _2)                                                \
    static int cmd_ ## Name ## _complete(struct list *l, int i) {       \
        if (i > 2)                                                      \
            return 1;                                                   \
        int cmd_ ## Name ## _completions(struct list *l, int i);        \
        return cmd_ ## Name ## _completions(l, i);                      \
    }                                                                   \

#define X3(Name, _1, _2)                                                \
    static int cmd_ ## Name ## _complete(struct list *l, int i) {       \
        if (i > 3)                                                      \
            return 1;                                                   \
        int cmd_ ## Name ## _completions(struct list *l, int i);        \
        return cmd_ ## Name ## _completions(l, i);                      \
    }                                                                   \

    CMD_LIST_CONSOLE

#undef X0
#undef X1
#undef X2
#undef X3

/********* Init cmd list **********/

/**
 * Readline thread initializer
 *
 * Create the command list from all Command and CVARs
 */
void thread_readline_init(void)
{
    cmd_list = list_new((f_list_cmp) cmd_cmp,
                        (f_list_free) cmd_free);

    /* Add all commands */

#define X0(Name, Desc, Usage) X(Name, Desc, Usage)
#define X1(Name, Desc, Usage) X(Name, Desc, Usage)
#define X2(Name, Desc, Usage) X(Name, Desc, Usage)
#define X3(Name, Desc, Usage) X(Name, Desc, Usage)
#define X(Name, Desc, Usage) do {                                 \
        struct cmd *c = calloc(1, sizeof (struct cmd));           \
        c->name = #Name;                                          \
        c->parse = cmd_ ## Name ## _parse;                        \
        c->complete = cmd_ ## Name ## _complete;                  \
        c->usage = Usage;                                         \
        c->description = Desc;                                    \
        list_add(cmd_list, c);                                    \
    } while (0);

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

        /* Add all CVARs */

#define X(Name) do {                                              \
        struct cmd *c = calloc(1, sizeof (struct cmd));           \
        c->name = #Name;                                          \
        c->parse = cvar_parse;                                    \
        c->complete = cvar_complete;                              \
        c->usage = NULL;                                          \
        c->description = NULL;                                    \
        list_add(cmd_list, c);                                    \
    } while (0);
#define XINT(Name, DefaultValue) X(Name)
#define XSTR(Name, DefaultValue) X(Name)
#define XBOOL(Name, DefaultValue) X(Name)

    CVAR_LIST

#undef X
#undef XINT
#undef XSTR
#undef XBOOL

    list_rl_init(cmd_list,
                 (f_list_rl_match) cmd_match,
                 (f_list_rl_copy) cmd_copy);
}

/********* Auto completion *********/

/**
 * Dummy completion routine to prevent the default completion behavior
 * enumerating files
 */
static char *wb_completion_entry(const char *text, int end)
{
    return NULL;
}

/**
 * Custom completion dispatch routine.
 * If start = 0 it means we are completing the Command of CVAR name.
 * Else, we are completing the Command arguments.
 *
 * Command/CVARs name completion is a statically allocated list while the
 * Command argument completions are done using a dynamic one.
 *
 * Since the Readline completion system does not allow us to pass a pointer to
 * arbitrary data to the completion routine, we use a static variable defined
 * in list.c to save the current completion list.
 */
static char **wb_completion(const char *text, int start, int end)
{
    rl_completion_entry_function = wb_completion_entry;

    /* Completing command nmame */
    if (start == 0)
    {
        list_rl_set(cmd_list);

        return rl_completion_matches(text, &list_rl_generator);
    }

    /* Completing arguments */
    else
    {
        struct cmd *current_cmd = NULL;

        size_t cmd_name_len = strcspn(rl_line_buffer, " \t");

        if (cmd_name_len > 0)
        {
            char *cmd_name = calloc(1, cmd_name_len + 1);

            strncpy(cmd_name, rl_line_buffer, cmd_name_len);

            current_cmd = list_get(cmd_list, cmd_name);

            free(cmd_name);
        }

        /* We failed at recognizing the command */
        if (current_cmd == NULL)
        {
            /* Not a problem, just no completions */
            return NULL;
        }

        /* Try to guess current argument index from cursor pos */
        int arg_index = 0;
        {
            int pos = cmd_name_len;

            while (pos < rl_point)
            {
                pos += strspn(rl_line_buffer + pos, " \t");
                pos += strcspn(rl_line_buffer + pos, " \t");
                ++arg_index;
            }
        }

        struct list *list = list_new((f_list_cmp) strcmp,
                                     (f_list_free) free);

        if (!current_cmd->complete(list, arg_index))
        {
            list_free(list);
            rl_completion_entry_function = NULL;

            return NULL;
        }

        list_rl_init(list,
                     (f_list_rl_match) strncmp,
                     (f_list_rl_copy) strdup);

        list_rl_set(list);

        char **matches =
            rl_completion_matches(text, &list_rl_generator);

        list_free(list);

        return matches;
    }
}

#ifndef __MINGW32__
static sigjmp_buf readline_loop;

/**
 * SIGINT readline handler
 * When we hit Ctrl-C, we will land here.
 */
static void readline_sigint_handler(int signum)
{
    /* Jump back to return point */
    siglongjmp(readline_loop, -1);
}
#endif /* !__MINGW32__ */

/********* Readline *********/

/**
 * Readline cleanup handler
 */
static void thread_readline_close(void *vargs)
{
    /* If we can, reset terminal output */
    if (rl_deprep_term_function != NULL)
        rl_deprep_term_function();

    if (cmd_list != NULL)
    {
        list_free(cmd_list);
        cmd_list = NULL;
    }

    printf("\n");
}

/**
 * Readline thread entry point.
 *
 * When compiled using the DBus API, we need the readline to be an individual
 * thread. Otherwise, for a full featured/safe readline, we need it to be used
 * as a function in the main thread.
 */
void *thread_readline(void *vargs)
{
#ifdef DBUS_API
    struct thread *t = (struct thread *) vargs;

    pthread_cleanup_push(thread_readline_close, t);
#endif /* DBUS_API */

    using_history();

    rl_catch_signals = 1;
    rl_bind_key('\t', rl_complete);
    rl_attempted_completion_function = wb_completion;
    rl_completion_entry_function = NULL;

#ifndef __MINGW32__
    signal(SIGINT, readline_sigint_handler);

    /* Set SIGINT return point */
    while (sigsetjmp(readline_loop, 1) != 0)
    {
        /*
         * If we land here, it means we caught a SIGINT.
         * Clear readline and reset return point.
         */
        printf("\n");
    }
#endif /* !__MINGW32__ */

    while (session.state != STATE_DEAD)
    {
        char *buff_readline = readline("CMD# ");

        if (buff_readline == NULL)
        {
            break;
        }

        if (buff_readline[0] != '\0' && buff_readline[1] != '\0')
        {
            add_history(buff_readline);

            parse_cmd(buff_readline);
        }

        free(buff_readline);
    }

    /*
     * If we are here, it means user did a Ctrl+D to exit readline. This is a
     * desired exit for the DBus-API.
     */
    if (session.state != STATE_DEAD)
    {
#ifdef DBUS_API
        dbus_api_quit(1);
#endif /* DBUS_API */
        session.state = STATE_DEAD;
    }

#ifdef DBUS_API
    pthread_cleanup_pop(1);

    return thread_close(t);
#else /* DBUS_API */
    thread_readline_close(NULL);

    return NULL;
#endif /* DBUS_API */
}
