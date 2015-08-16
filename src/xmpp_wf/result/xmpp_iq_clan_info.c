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

#include <wb_tools.h>
#include <wb_stream.h>
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>
#include <wb_session.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void xmpp_iq_clan_info_cb(const char *msg_id,
                                 const char *msg,
                                 void *args)
{
    char *data = wf_get_query_content(msg);

    clanmate_list_empty();

    /* Answer:
       <clan name="XXXXXXX" clan_id="xxx" description="..."
             creation_date="xxxx" master="xxxx" clan_points="xxxxx"
             members="xx" master_badge="xxxx" master_stripe="xxxx"
             master_mark="xxx" leaderboard_position="xx">
    */

    const char *m = strstr(data, "<clan ");

    if (m != NULL)
    {
        m += sizeof ("<clan ") - 1;

        session.clan_id = get_info_int(m, "clan_id='", "'", NULL);
        session.clan_name = get_info(m, "name='", "'", NULL);

        /* Nodes:
           <clan_member_info nickname="xxxx" profile_id="xxx"
                experience="xxx" clan_points="xxx"
                invite_date="xxxxxxxxx" clan_role="3"
                jid="xxxx@warface/GameClient" status="1" />
        */

        while ((m = strstr(m, "<clan_member_info ")))
        {
            m += sizeof ("<clan_member_info ");

            char *jid = get_info(m, "jid='", "'", NULL);
            char *nick = get_info(m, "name='", "'", "CLANMATE NICK");
            char *pid = get_info(m, "profile_id='", "'", NULL);
            int status = get_info_int(m, "status='", "'", NULL);
            int exp = get_info_int(m, "experience='", "'", NULL);
            int cp = get_info_int(m, "clan_points='", "'", NULL);
            int cr = get_info_int(m, "clan_role='", "'", NULL);

            if (strcmp(session.nickname, nick) != 0)
            {
                clanmate_list_add(jid, nick, pid, status, exp, cp, cr);

                if (jid && *jid)
                    xmpp_iq_peer_clan_member_update(jid);
            }
            else
            {
                session.clan_points = cp;
                session.clan_role = cr;
                session.clan_joined = get_info_int(m, "invite_date='", "'", NULL);
            }

            free(jid);
            free(nick);
            free(pid);
        }
    }

    printf("Clan member count: %ld/50\n", session.clanmates->length);

    free(data);
}

void xmpp_iq_clan_info_r(void)
{
    qh_register("clan_info", 1, xmpp_iq_clan_info_cb, NULL);
}
