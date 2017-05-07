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
#include <wb_option.h>
#include <wb_stream.h>
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>
#include <wb_session.h>
#include <wb_threads.h>
#include <wb_lang.h>
#include <wb_tools.h>

int main(int argc, char *argv[])
{
    const char *exe_path = argv[0];

    cvar_init();

    cvar_parse_file("wb.cfg");

    if (cvar.g_language != NULL)
    {
        char *path;

        FORMAT(path, "cfg/lang/%s.cfg", cvar.g_language);
        cvar_parse_file(path);
        free(path);
    }

    char *token = NULL;
    char *online_id = NULL;
    char *cmdline = NULL;

    option_parse(argc, argv, &token, &online_id, &cmdline);

    /* Start of -- Legal Notices */

    if (isatty(STDOUT_FILENO))
    {
        xprintf(
            "\nWarfacebot Copyright (C) 2015-2017 Levak Borok\n"
            "This program comes with ABSOLUTELY NO WARRANTY.\n"
            "This is free software, and you are welcome to redistribute it\n"
            "under certain conditions; see AGPLv3 Terms for details.\n\n");
    }

    /* End of -- Legal Notices */

    idh_init();

    XMPP_REGISTER_QUERY_HDLR();
    XMPP_WF_REGISTER_QUERY_HDLR();

    int wfs = stream_connect(cvar.online_server,
                             cvar.online_server_port);

    if (wfs > 0)
    {
        session_init(wfs, online_id);

        threads_init();

        xmpp_connect(token, online_id);

        threads_run(exe_path, cmdline);

        threads_quit();

        xmpp_close();

        session_free();
    }

    free(token);
    free(online_id);
    free(cmdline);

    xprintf("%s", LANG(wb_closing));

    cvar_free();

    return 0;
}
