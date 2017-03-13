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

#include <wb_tools.h>
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>
#include <wb_session.h>
#include <wb_list.h>
#include <wb_log.h>
#include <wb_lang.h>

#include <stdlib.h>
#include <string.h>

void clan_process_node(const char *data)
{
    const char *m = data;

    /* Reset */
    clanmate_list_empty();
    session.profile.clan.id = 0;
    free(session.profile.clan.name);
    session.profile.clan.name = NULL;

    /* Clan node:
       <clan name="XXXXXXX" clan_id="xxx" description="..."
             creation_date="xxxx" master="xxxx" clan_points="xxxxx"
             members="xx" master_badge="xxxx" master_stripe="xxxx"
             master_mark="xxx" leaderboard_position="xx">
    */

    if (m != NULL)
    {
        m += sizeof ("<clan ") - 1;

        session.profile.clan.id = get_info_int(m, "clan_id='", "'", NULL);

        free(session.profile.clan.name);
        session.profile.clan.name = get_info(m, "name='", "'", NULL);

        session.profile.clan.master.badge =
            get_info_int(data, "master_badge='", "'", NULL);
        session.profile.clan.master.stripe =
            get_info_int(data, "master_stripe='", "'", NULL);
        session.profile.clan.master.mark =
            get_info_int(data, "master_mark='", "'", NULL);

        /* Clan member nodes:
           <clan_member_info nickname="xxxx" profile_id="xxx"
                experience="xxx" clan_points="xxx"
                invite_date="xxxxxxxxx" clan_role="3"
                jid="xxxx@warface/GameClient" status="1" />
        */

        while ((m = strstr(m, "<clan_member_info")))
        {
            char *info = get_info(m, "<clan_member_info", "/>", NULL);

            char *jid = get_info(info, "jid='", "'", NULL);
            char *nick = get_info(info, "nickname='", "'", NULL);
            char *pid = get_info(info, "profile_id='", "'", NULL);
            int status = get_info_int(info, "status='", "'", NULL);
            int exp = get_info_int(info, "experience='", "'", NULL);
            int cp = get_info_int(info, "clan_points='", "'", NULL);
            int cr = get_info_int(info, "clan_role='", "'", NULL);

            if (strcmp(session.profile.nickname, nick) != 0)
            {
                clanmate_list_update(jid, nick, pid, status, exp, cp, cr);

                xprintf("%s: \033[1;%dm%s\033[0m",
                        LANG(console_clanmate),
                        status & STATUS_ONLINE ? 32 : 31, nick);
            }
            else
            {
                session.profile.clan.points = cp;
                session.profile.clan.role = cr;
                session.profile.clan.joined =
                    get_info_int(m, "invite_date='", "'", NULL);
            }

            free(jid);
            free(nick);
            free(pid);
            free(info);
            ++m;
        }

        list_foreach(session.profile.clanmates,
                     (f_list_callback) xmpp_iq_peer_clan_member_update_clanmate,
                     NULL);

        xprintf("%s: %ld/50",
                LANG(console_clanmates),
                session.profile.clanmates->length);
    }
    else
    {
        xprintf("%s", LANG(error_not_in_a_clan));
    }
}

static void xmpp_iq_clan_info_cb(const char *msg_id,
                                 const char *msg,
                                 void *args)
{
    /* Record clanmates to list
       <iq from='masterserver@warface/xxx' type='get'>
        <query xmlns='urn:cryonline:k01'>
         <clan_info>
          <clan ...>
           <clan_member_info .../>
           ...
          </clan>
         </clan_info>
        </query>
       </iq>
    */

    char *data = wf_get_query_content(msg);

    if (data == NULL)
        return;

    char *clan_node = get_info(data, "<clan ", "</clan>", NULL);

    clan_process_node(data);

    free(clan_node);
    free(data);
}

void xmpp_iq_clan_info_r(void)
{
    qh_register("clan_info", 1, xmpp_iq_clan_info_cb, NULL);
}
