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

#include <wb_log.h>
#include <wb_cvar.h>
#include <wb_lang.h>
#include <wb_tools.h>

#include <string.h>
#include <stdlib.h>
#include <getopt.h>

static struct option long_options[] = {
    {"help", no_argument, 0, 'h' },
    {"id", required_argument, 0, 'i' },
    {"token", required_argument, 0, 't' },
    {"define", required_argument, 0, 'd' },
    {"load-file", required_argument, 0, 'f' },
    {0, 0, 0, 0 }
};

static void print_usage(void)
{
    eprintf(
        "USAGE: ./wb  -i|--id <online_id>\n"
        "             -t|--token <token>\n"
        "            [-d|--define <name=value>]\n"
        "            [-f|--load-file <path>]\n"
        "            [-h|--help]\n"
        );
}

static void print_help(void)
{
    eprintf("\n"
        "  -i, --id <online_id>\t\tWarface ID\n"
        "  -t, --token <token>\t\tWarface token\n"
        "  -d, --define <name=value>\tSet a CVar\n"
        "  -f, --load-file <path>\tLoad CVar config file\n"
        "  -h, --help\t\t\tDisplay this help\n"
        );
}

static void define_cvar(const char *define)
{
    char *line = strdup(define);

    const char sep[] = "= \t";
    char *saveptr;
    char *name = strtok_r(line, sep, &saveptr);
    char *value = strtok_r(NULL, "", &saveptr);

    if (name == NULL || value == NULL)
    {
        free(line);

        return;
    }

    value += strspn(value, sep);

    cvar_set(name, value);

    free(line);
}

void option_parse(int argc, char *argv[],
                  char **token, char **online_id,
                  char **cmdline)
{
    int opt = 0;
    int long_index = 0;

    *cmdline = strdup("");

    while ((opt = getopt_long(
                argc, argv,"ht:i:d:f:s:",
                long_options, &long_index)) != -1)
    {
        switch (opt)
        {
            case 'd':
            {
                if (strstr(*cmdline, optarg) == NULL)
                {
                    char *s;
                    FORMAT(s, "%s -d '%s'", *cmdline, optarg);
                    free(*cmdline);
                    *cmdline = s;
                }

                define_cvar(optarg);
                break;
            }
            case 'f':
            {
                char *path = realpath(optarg, NULL);

                if (path != NULL && strstr(*cmdline, path) == NULL)
                {
                    char *s;
                    FORMAT(s, "%s -f '%s'", *cmdline, path);
                    free(*cmdline);
                    *cmdline = s;
                }

                switch (cvar_parse_file(path))
                {
                    case CVAR_PARSE_NOTFOUND:
                        eprintf("%s: %s",
                                LANG(error_file_notfound),
                                path);
                        break;
                    case CVAR_PARSE_ERROR:
                        eprintf("%s: %s",
                                LANG(error_parse_error),
                                path);
                        break;
                    default:
                        break;
                }

                free(path);
                break;
            }
            case 't':
                free(*token);
                *token = strdup(optarg);
                break;
            case 'i':
                free(*online_id);
                *online_id = strdup(optarg);
                break;
            case 'h':
                print_usage();
                print_help();
                exit(EXIT_SUCCESS);
                break;
            default:
                print_usage();
                exit(EXIT_FAILURE);
                break;
        }
    }

    if (*token == NULL)
    {
        eprintf("%s: `%s`",
                LANG(error_required_option),
                "--token");
        print_usage();
        exit(EXIT_FAILURE);
    }

    if (*online_id == NULL)
    {
        eprintf("%s: `%s`",
                LANG(error_required_option),
                "--id");
        print_usage();
        exit(EXIT_FAILURE);
    }

    if (cvar.online_server == NULL)
    {
        eprintf("%s: `%s`",
                LANG(error_required_cvar),
                "online_server");
        print_usage();
        exit(EXIT_FAILURE);
    }
}
