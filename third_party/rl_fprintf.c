#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <readline/readline.h>

/*
  Jean-Bernard Pellerin Nov. 7th, 2011
  http://stackoverflow.com/questions/8032418
*/

void rl_fprintf(FILE *f, char *fmt, ...)
{
    int need_hack = (rl_readline_state & RL_STATE_READCMD) > 0;
    char *saved_line = NULL;
    int saved_point = 0;
    va_list args;

    if (need_hack)
    {
        rl_save_prompt();

        if (rl_end > 0)
        {
            saved_point = rl_point;
            saved_line = rl_copy_text(0, rl_end);
            rl_replace_line("", 0);
        }

        rl_redisplay();
    }

    va_start(args, fmt);
    vfprintf(f, fmt, args);
    va_end(args);
    fprintf(f, "\n");

    if (need_hack)
    {
        rl_restore_prompt();

        if (saved_line != NULL)
        {
            rl_replace_line(saved_line, 0);
            rl_point = saved_point;
            free(saved_line);
        }

        rl_redisplay();
    }
}
