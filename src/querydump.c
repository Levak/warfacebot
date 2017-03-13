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

#include <wb_querydump.h>
#include <wb_tools.h>
#include <wb_cvar.h>
#include <wb_lang.h>
#include <wb_log.h>
#include <wb_session.h>
#include <wb_xml.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

void querydump_init(const char *online_id)
{
#ifdef DEBUG
    if (cvar.query_dump_to_file)
    {
        time_t     t = time(NULL);
        struct tm  ts;
        char       buf[64];

        char *fname;

        localtime_r(&t, &ts);

        strftime(buf, sizeof (buf), "%Y-%m-%d_%H-%M-%S", &ts);

        FORMAT(fname,
               "%s%s-%s-%s.xml",
               cvar.query_dump_location,
               online_id,
               cvar.game_server_name,
               buf);

        if (MKDIR(cvar.query_dump_location))
        {
            if (errno != EEXIST)
            {
                eprintf("%s %s",
                        LANG(error_create_directory),
                        cvar.query_dump_location);
            }
        }

        session.log_file = fopen(fname, "w");

        if (session.log_file != NULL)
        {
            fprintf(session.log_file,
                    "<abstract-root"
                    " xmlns:stream=\"http://etherx.jabber.org/streams\">\n");
        }
        else
        {
            eprintf("%s: %s",
                    LANG(error_write_file),
                    fname);
        }

        free(fname);
    }
#endif /* DEBUG */
}

void querydump_free(void)
{
#ifdef DEBUG
    if (session.log_file != NULL)
    {
        fprintf(session.log_file, "</abstract-root>\n");
        fclose(session.log_file);
        session.log_file = NULL;
    }
#endif /* DEBUG */
}

void querydump_emit(const char *comment, const char *query)
{
#ifdef DEBUG
    if (session.log_file == NULL || query == NULL || comment == NULL)
        return;

    time_t     t = time(NULL);
    struct tm  ts;
    char       buf[64];

    localtime_r(&t, &ts);

    strftime(buf, sizeof (buf), "%Y-%m-%d %H:%M:%S %Z", &ts);

    if (strstr(query, "stream:stream") == NULL)
    {
        fprintf(session.log_file,
                "<!-- %s %s --> %s\n",
                comment,
                buf,
                query);
    }
    else
    {
        fprintf(session.log_file,
                "<!-- %s %s - %s -->\n",
                comment,
                buf,
                query);
    }
#endif /* DEBUG */
}

void querydump_comment(const char *comment)
{
#ifdef DEBUG
    if (session.log_file == NULL || comment == NULL)
        return;

    time_t     t = time(NULL);
    struct tm  ts;
    char       buf[64];

    localtime_r(&t, &ts);

    strftime(buf, sizeof (buf), "%Y-%m-%d %H:%M:%S %Z", &ts);

    fprintf(session.log_file,
            "<!-- COMM %s - %s -->\n",
            buf,
            comment);

#endif /* DEBUG */
}
