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

	//clanmate_list_empty();
	unsigned int num_clanmates = session.clanmates->length;

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
		int old_leaderboard_position = session.clan_leaderboard_position;
		session.clan_leaderboard_position = get_info_int(m, "leaderboard_position='", "'", NULL);
		if (old_leaderboard_position != session.clan_leaderboard_position)
			LOGPRINT("%-20s " BOLD "%d\n", "CLAN RANK", session.clan_leaderboard_position);

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
            char *nick = get_info(m, "name='", "'", NULL);
            char *pid = get_info(m, "profile_id='", "'", NULL);
            int status = get_info_int(m, "status='", "'", NULL);
            int exp = get_info_int(m, "experience='", "'", NULL);
            int cp = get_info_int(m, "clan_points='", "'", NULL);
            int cr = get_info_int(m, "clan_role='", "'", NULL);
			unsigned int invite_date = get_info_int(m, "invite_date='", "'", NULL);


            if (strcmp(session.nickname, nick) != 0)
            {
				if (list_get(session.clanmates, nick))
					clanmate_list_update(jid, nick, pid, status, exp, cp, cr, invite_date);
				else
				{
					clanmate_list_add(jid, nick, pid, status, exp, cp, cr, invite_date);

					if (jid && *jid)
						if (!(status & (STATUS_AFK | STATUS_PLAYING)))
							LOGPRINT("Clanmate: " KGRN BOLD "%s" KWHT "\n", nick);
						else if (status & STATUS_PLAYING)
							LOGPRINT("Clanmate: " KMAG BOLD "%s" KWHT "\n", nick);
						else
							LOGPRINT("Clanmate: " KYEL BOLD "%s" KWHT "\n", nick);
					else
						LOGPRINT("Clanmate: " KCYN BOLD "%s" KWHT "\n", nick);
				}

                if (jid && *jid)
                    xmpp_iq_peer_clan_member_update(jid);
            }
            else
            {
                session.clan_points = cp;
                session.clan_role = cr;
                session.clan_joined = invite_date;
            }

            free(jid);
            free(nick);
            free(pid);
        }
    }

	if (num_clanmates != session.clanmates->length)
		LOGPRINT("Clan member count: " KWHT BOLD "%u/50\n", session.clanmates->length);

    free(data);
}

void xmpp_iq_clan_info_r(void)
{
    qh_register("clan_info", 1, xmpp_iq_clan_info_cb, NULL);
}
