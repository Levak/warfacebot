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

#include <wb_cvar.h>
#include <wb_log.h>
#include <wb_tools.h>

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct cvar cvar = { 0 };

enum e_cvar_type
{
    CVAR_INT,
    CVAR_STR,
    CVAR_BOOL,
};

struct cvar_assoc
{
    const enum e_cvar_type type;
    const void *ptr;
    const char name[32];
} cvar_assoc[] =
{
#define XINT(Name, DefaultValue) { CVAR_INT, &cvar.Name, #Name },
#define XSTR(Name, DefaultValue) { CVAR_STR, &cvar.Name, #Name },
#define XBOOL(Name, DefaultValue) { CVAR_BOOL, &cvar.Name, #Name },
    CVAR_LIST
#undef XINT
#undef XSTR
#undef XBOOL
};

char *cvar_copy(const char *val)
{
    return (val != NULL) ? strdup(val) : NULL;
}

void cvar_init(void)
{
#define XINT(Name, DefaultValue) cvar.Name = DefaultValue;
#define XSTR(Name, DefaultValue) {                      \
        t_cvar_str *p = (t_cvar_str *) &cvar.Name;      \
        *p = cvar_copy(DefaultValue);                   \
    }
#define XBOOL(Name, DefaultValue) cvar.Name = DefaultValue;
    CVAR_LIST
#undef XINT
#undef XSTR
#undef XBOOL
}

void cvar_free(void)
{
#define XINT(Name, DefaultValue)
#define XSTR(Name, DefaultValue) {                      \
        t_cvar_str *p = (t_cvar_str *) &cvar.Name;      \
        free(*p), *p = NULL;                            \
    }
#define XBOOL(Name, DefaultValue)
    CVAR_LIST
#undef XINT
#undef XSTR
#undef XBOOL
}

static struct cvar_assoc *cvar_get(const char *key)
{
    for (unsigned i = 0;
         i < sizeof (cvar_assoc) / sizeof (cvar_assoc[0]);
         ++i)
    {
        if (strcmp(key, cvar_assoc[i].name) == 0)
            return &cvar_assoc[i];
    }

    return NULL;
}

int cvar_set(const char *name, const char *value)
{
    if (name == NULL)
        return 0;

    struct cvar_assoc *c = cvar_get(name);

    if (c == NULL)
        return 0;

    switch(c->type)
    {
        case CVAR_INT:
        {
            t_cvar_int *p = (t_cvar_int *) c->ptr;

            if (value != NULL)
            {
                if (value[0] == '0'
                    && (value[1] == 'x' || value[1] == 'X'))
                    *p = strtoll(value + 2, NULL, 16);
                else
                    *p = strtoll(value, NULL, 10);
            }

            xprintf("%s = %ld\n", name, *p);
            return 1;
        }
        case CVAR_BOOL:
        {
            t_cvar_bool *p = (t_cvar_bool *) c->ptr;

            if (value != NULL)
            {
                if (0 == strcasecmp(value, "true"))
                    *p = 1;
                else if (0 == strcasecmp(value, "false"))
                    *p = 0;
                else
                    *p = strtol(value, NULL, 10) ? 1 : 0;
            }

            xprintf("%s = %d\n", name, *p);
            return 1;
        }
        case CVAR_STR:
        {
            t_cvar_str *p = (t_cvar_str *) c->ptr;

            if (value != NULL)
            {
                free(*p);
                *p = get_trim(value);

                if (!*p[0])
                {
                    free(*p);
                    *p = NULL;
                }
            }

            xprintf("%s = %s\n", name, *p);
            return 1;
        }
        default:
            return 0;
    }
}

enum e_cvar_parse cvar_parse_file(const char *path)
{
    FILE *f = fopen(path, "r");

    if (f == NULL)
        return CVAR_PARSE_NOTFOUND;

    char *line = NULL;
    size_t len = 0;

    while (getline(&line, &len, f) != -1)
    {
        const char sep[] = "= \t";
        char *saveptr;
        char *name = strtok_r(line, sep, &saveptr);
        char *value = strtok_r(NULL, "", &saveptr);

        if (name == NULL)
        {
            free(line);

            return CVAR_PARSE_ERROR;
        }
        else if (value == NULL)
        {
            continue;
        }

        value += strspn(value, sep);

        cvar_set(name, value);
    }

    free(line);
    fclose(f);

    return CVAR_PARSE_SUCCESS;
}
