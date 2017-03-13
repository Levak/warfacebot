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

#include <wb_cvar.h>
#include <wb_lang.h>
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
    LANG_STR,
};

struct cvar_assoc
{
    const enum e_cvar_type type;
    const void *ptr;
    const char name[80];
} cvar_assoc[] =
{
#define XINT(Name, DefaultValue) { CVAR_INT, &cvar.Name, #Name },
#define XSTR(Name, DefaultValue) { CVAR_STR, &cvar.Name, #Name },
#define XBOOL(Name, DefaultValue) { CVAR_BOOL, &cvar.Name, #Name },
#define XLANG(Name) { LANG_STR, &lang.Name, #Name },
    CVAR_LIST
    LANG_LIST
#undef XLANG
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
#undef XLANG
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
#define XLANG(Name) {                                           \
        t_cvar_str *p = (t_cvar_str *) &lang.Name.value;        \
        int *i = (int *) &lang.Name.is_set;                     \
        if (*i)                                                 \
            free(*p), *p = NULL, *i = 0;                        \
    }
    CVAR_LIST
    LANG_LIST
#undef XLANG
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

static char *parse_string(const char *value)
{
    char *t = get_trim(value);

    /* Skip double quotes if any */
    size_t len = strlen(t);
    if (t[0] == '"' && t[len - 1] == '"')
    {
        char *t2 = strdup(t + 1);

        t2[len - 2] = 0;
        free(t);
        t = t2;
    }

    return t;
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

            xprintf("%s = %ld", name, *p);
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

            xprintf("%s = %d", name, *p);
            return 1;
        }
        case CVAR_STR:
        {
            t_cvar_str *p = (t_cvar_str *) c->ptr;

            if (value != NULL)
            {
                free(*p);
                *p = parse_string(value);
            }

            xprintf("%s = %s", name, *p);
            return 1;
        }
        case LANG_STR:
        {
            if (value != NULL)
            {
                s_lang_assoc *a = (s_lang_assoc *) c->ptr;
                t_cvar_str *p = (t_cvar_str *) &a->value;
                int *i = (int *) &a->is_set;

                if (*i)
                    free(*p);
                *p = parse_string(value);
                *i = 1;
            }

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
        /* Skip comments */
        if (line[0] == '#')
            continue;

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
