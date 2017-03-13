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

#include <stdlib.h>
#include <string.h>

static char *xml_serialize_(const char *str, int inplace)
{
    size_t count_amp = 0;
    size_t count_lt = 0;
    size_t count_gt = 0;
    size_t count_apos = 0;
    size_t count_quot = 0;
    size_t in_size = 0;

    { /* Find the number of each char to replace */
        const char *s = str;

        for (; *s; ++s)
        {
            switch (*s)
            {
                case '&': ++count_amp; break;
                case '<': ++count_lt; break;
                case '>': ++count_gt; break;
                case '\'': ++count_apos; break;
                case '"': ++count_quot; break;
                default: break;
            }

            ++in_size;
        }
    }

    /* Compute the space we need for the new string */
    size_t out_size = in_size +
        4 * count_amp +
        3 * count_lt +
        3 * count_gt +
        5 * count_apos +
        5 * count_quot;

    char *out = NULL;

    if (inplace)
    {
        str = realloc((char *) str, out_size + 1);
        out = (char *) str;
    }
    else
        out = malloc(out_size + 1);

    /* Don't forget null terminator */
    out[out_size] = 0;

    { /* Copy str to out while escaping special chars */
        const char *s = str + in_size - 1;
        char *o = out + out_size - 1;

        for (; s >= str; --s)
        {
            switch (*s)
            {
                case '&':
                    *(o--) = ';';
                    *(o--) = 'p';
                    *(o--) = 'm';
                    *(o--) = 'a';
                    *(o--) = '&';
                    break;
                case '<':
                    *(o--) = ';';
                    *(o--) = 't';
                    *(o--) = 'l';
                    *(o--) = '&';
                    break;
                case '>':
                    *(o--) = ';';
                    *(o--) = 't';
                    *(o--) = 'g';
                    *(o--) = '&';
                    break;
                case '\'':
                    *(o--) = ';';
                    *(o--) = 's';
                    *(o--) = 'o';
                    *(o--) = 'p';
                    *(o--) = 'a';
                    *(o--) = '&';
                    break;
                case '"':
                    *(o--) = ';';
                    *(o--) = 't';
                    *(o--) = 'o';
                    *(o--) = 'u';
                    *(o--) = 'q';
                    *(o--) = '&';
                    break;
                default:
                    *(o--) = *s;
                    break;
            }
        }
    }

    return out;
}

char *xml_serialize(const char *str)
{
    return xml_serialize_(str, 0);
}

char *xml_serialize_inplace(char **str)
{
    char *s = xml_serialize_(*str, 1);
    *str = s;

    return s;
}

static char *xml_deserialize_(const char *str, int inplace)
{
    char *out = inplace ? (char *) str : malloc(strlen(str) + 1);

    const char *s = str;
    char *o = out;

    for (; *s; ++s, ++o)
    {
        switch (*s)
        {
            case '&':
                if (strncmp(s, "&amp;", 5) == 0)
                    *o = '&', s += 4;
                else if (strncmp(s, "&lt;", 4) == 0)
                    *o = '<', s += 3;
                else if (strncmp(s, "&gt;", 4) == 0)
                    *o = '>', s += 3;
                else if (strncmp(s, "&apos;", 6) == 0)
                    *o = '\'', s += 5;
                else if (strncmp(s, "&quot;", 6) == 0)
                    *o = '"', s += 5;
                else
                    *o = *s;
                break;
            default:
                *o = *s;
                break;
        }
    }

    *o = 0;

    if (inplace)
        out = realloc((char *) str, o - out + 1);

    return out;
}

char *xml_deserialize(const char *str)
{
    return xml_deserialize_(str, 0);
}

char *xml_deserialize_inplace(char **str)
{
    char *s = xml_deserialize_(*str, 1);
    *str = s;

    return s;
}
