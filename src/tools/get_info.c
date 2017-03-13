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
#include <wb_lang.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char *get_info(const char *input,
               const char *patt_b,
               const char *patt_e,
               const char *desc)
{
    int size_b = strlen(patt_b);
    char *ret = NULL;
    char *start = NULL;
    char *end = NULL;

    start = strstr(input, patt_b);

    if (start)
        end = strstr(start + size_b, patt_e);

    if (start && end)
    {
        start += size_b;
        ret = calloc(end - start + 1, 1);
        strncpy(ret, start, end - start);
        ret[end - start] = 0;
        if (desc)
            xprintf("%s: %s", desc, ret);
    }
    else if (desc)
        eprintf("%s: %s", LANG(error_get_info), desc);

    return ret;
}

char *get_info_first(const char *input,
                     const char *patt_b,
                     const char *patt_e_list,
                     const char *desc)
{
    int size_b = strlen(patt_b);
    char *ret = NULL;
    char *start = NULL;
    char *end = NULL;
    const char *patt_e = patt_e_list;

    start = strstr(input, patt_b);

    if (start)
    {
        for (; *patt_e; ++patt_e)
        {
            char *nend = strchr(start + size_b, *patt_e);
            if (!end)
                end = nend;
            else if (nend && nend < end)
                end = nend;
        }
    }

    if (start && end)
    {
        start += size_b;
        ret = calloc(end - start + 1, 1);
        strncpy(ret, start, end - start);
        ret[end - start] = 0;
        if (desc)
            xprintf("%s: %s", desc, ret);
    }
    else if (desc)
        eprintf("%s: %s", LANG(error_get_info), desc);

    return ret;
}

long long int get_info_int(const char *input,
                      const char *patt_b,
                      const char *patt_e,
                      const char *desc)
{
    char *b = strstr(input, patt_b);
    char *e = NULL;
    long long int ret = 0;

    if (b != NULL)
    {
        b += strlen(patt_b);
        e = strstr(b, patt_e);
    }

    if (b != NULL && e != NULL)
    {
        ret = strtoll(b, &e, 10);

        if (desc)
            xprintf("%s: %lli", desc, ret);
    }
    else if (desc)
        eprintf("%s: %s", LANG(error_get_info), desc);

    return ret;
}

float get_info_float(const char *input,
                     const char *patt_b,
                     const char *patt_e,
                     const char *desc)
{
    char *b = strstr(input, patt_b);
    char *e = NULL;
    float ret = 0.0f;

    if (b != NULL)
    {
        b += strlen(patt_b);
        e = strstr(b, patt_e);
    }

    if (b != NULL && e != NULL)
    {
        ret = strtof(b, &e);

        if (desc)
            xprintf("%s: %f", desc, ret);
    }
    else if (desc)
        eprintf("%s: %s", LANG(error_get_info), desc);

    return ret;
}

char *get_token(char *str, const char *delim, char **saveptr)
{
    char *ret = strtok_r(str, delim, saveptr);

    if (ret != NULL)
        return strdup(ret);
    else
        return NULL;
}

char* get_trim(const char* s)
{
    int start;
    int end;

    for (start = 0; s[start] && isspace(s[start]); ++start)
        continue;

    if (!s[start])
        return strdup("");

    for (end = strlen(s); end > 0 && isspace(s[end - 1]); --end)
        continue;

    char *ret = strdup(s + start);

    ret[end - start] = 0;

    return ret;
}
