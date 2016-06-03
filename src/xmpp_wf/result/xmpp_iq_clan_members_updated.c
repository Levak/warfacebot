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
#include <wb_list.h>

#include <stdlib.h>
#include <wb_log.h>
#include <string.h>

static void xmpp_iq_clan_members_updated_cb(const char *msg_id,
                                 const char *msg,
                                 void *args)
{
    /* Answer:
       <iq from='k01.warface' type='get'>
        <query xmlns='urn:cryonline:k01'>
         <clan_members_updated>
          <update profile_id='xxxx'>
           <clan_member_info nickname='xxxx' profile_id='xxxx'
                 experience='xxxxx' clan_points='xxxx' invite_date='xxxx'
                 clan_role='xxx' jid='xxxxx' status='xxxx'/>
          </update>
         </clan_members_updated>
        </query>
       </iq>

       or

       <iq from='k01.warface' type='get'>
        <query xmlns='urn:cryonline:k01'>
         <clan_members_updated>
          <update profile_id='xxxx'/>
         </clan_members_updated>
        </query>
       </iq>
    */

    char *data = wf_get_query_content(msg);

    if (data == NULL)
        return;

    const char *m = data;

    while ((m = strstr(m, "<update")))
    {
        char *update = get_info(m, "<update", "</update>", NULL);

        if (update == NULL)
            update = get_info(m, "<update", "/>", NULL);

        char *jid = get_info(update, "jid='", "'", NULL);
        char *nick = get_info(update, "nickname='", "'", NULL);
        char *pid = get_info(update, "profile_id='", "'", NULL);
        int status = get_info_int(update, "status='", "'", NULL);
        int exp = get_info_int(update, "experience='", "'", NULL);
        int cp = get_info_int(update, "clan_points='", "'", NULL);
        int cr = get_info_int(update, "clan_role='", "'", NULL);

        /* If it's us */
        if (pid != NULL
            && strcmp(pid, session.profile.id) == 0)
        {
            session.profile.clan.points = cp;
            session.profile.clan.role = cr;
        }
        else
        {
            char *display_nick = NULL;

            if (nick == NULL)
            {
                struct clanmate *c =
                    list_get(session.profile.clanmates, pid);

                if (c != NULL)
                    display_nick = strdup(c->nickname);
            }
            else
                display_nick = strdup(nick);

            enum clan_update ret =
                clanmate_list_update(jid, nick, pid, status, exp, cp, cr);

            switch (ret)
            {
                case CLAN_UPDATE_JOINED:
                    xprintf("%s joined the clan\n", display_nick);
                    break;
                case CLAN_UPDATE_LEFT:
                    xprintf("%s left the clan\n", display_nick);
                    break;
                default:
                    break;
            }

            free(display_nick);
        }

        free(jid);
        free(nick);
        free(pid);
        free(update);
        ++m;
    }

    free(data);
}

void xmpp_iq_clan_members_updated_r(void)
{
    qh_register("clan_members_updated", 1, xmpp_iq_clan_members_updated_cb, NULL);
}
