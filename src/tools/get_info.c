/**
 * WarfaceBot, a blind XMPP client for Warface (FPS)
 * Copyright (C) 2015 Levak Borok <levak92@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wb_tools.h>
#include <wb_xml.h>

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
		if ( desc )
		{
			char *temp = xml_deserialize ( ret );
			LOGPRINT ( "%-16s "BOLD"%s\n"KRST, desc, temp );
			free ( temp );
		}
    }
    else if (desc)
        fprintf(stderr, "Could not find %s\n", desc);

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
			LOGPRINT ( "%s is %s\n", desc, ret );
    }
    else if (desc)
        fprintf(stderr, "Could not find %s\n", desc);

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
			LOGPRINT ( "%-16s %lli\n", desc, ret );
    }
    else if (desc)
        fprintf(stderr, "Could not find %s\n", desc);

    return ret;
}
