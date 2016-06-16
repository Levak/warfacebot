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

#include <wb_tools.h>
#include <wb_stream.h>
#include <wb_session.h>
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>
#include <wb_cmd.h>

#include <stdlib.h>
#include <wb_log.h>

struct cb_args
{
    f_cmd_stats_cb cb;
    void *args;
};

static void cmd_stats_cb(const char *msg,
                         enum xmpp_msg_type type,
                         void *args)
{
    /* Answer:
      <iq from='k01.warface' type='result'>
       <query xmlns='urn:cryonline:k01'>
        <get_master_servers>
         <masterservers>
          <server resource='pvp_newbie_1' server_id='101' channel='pvp_newbie'
                  rank_group='all' load='0.071765' online='61' min_rank='1'
                  max_rank='12' bootstrap=''>
           <load_stats>
            <load_stat type='quick_play' value='240'/>
            <load_stat type='survival' value='255'/>
            <load_stat type='pve' value='255'/>
           </load_stats>
          </server>
          <server resource='pvp_pro_4' ...
         </masterservers>
        </get_master_servers>
       </query>
      </iq>
     */

    struct cb_args *a = (struct cb_args *) args;

    if (type ^ XMPP_TYPE_ERROR)
    {
        const char *m = msg;

        while ((m = strstr(m, "<server")))
        {
            char *server = get_info(m, "<server", "</server>", NULL);
            char *resource = get_info(server, "resource='", "'", NULL);
            int online = get_info_int(server, "online='", "'", NULL);

            if (a->cb && resource != NULL)
                a->cb(resource, online, a->args);

            free(resource);
            free(server);
            ++m;
        }
    }

    if (a->cb)
        a->cb(NULL, 0, a->args);

    free(a);
}

void cmd_stats(f_cmd_stats_cb cb, void *args)
{
    t_uid id;
    struct cb_args *a = calloc(1, sizeof (struct cb_args));

    a->cb = cb;
    a->args = args;

    idh_generate_unique_id(&id);
    idh_register(&id, 0, cmd_stats_cb, a);

    send_stream_format(session.wfs,
                       "<iq to='k01.warface' type='get' id='%s'>"
                       "<query xmlns='urn:cryonline:k01'>"
                       "<get_master_servers/>"
                       "</query>"
                       "</iq>", &id);
}

void cmd_stats_console_cb(const char *resource, int online, void *args)
{
    if (resource != NULL)
        xprintf(" - %s: \t%i\n", resource, online);
}
