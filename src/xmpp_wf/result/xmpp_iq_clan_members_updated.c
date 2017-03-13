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

        if (update == NULL)
        {
            ++m;
            continue;
        }

        char *jid = get_info(update, "jid='", "'", NULL);
        char *nick = get_info(update, "nickname='", "'", NULL);
        char *pid = get_info(update, "profile_id='", "'", NULL);
        int status = get_info_int(update, "status='", "'", NULL);
        int exp = get_info_int(update, "experience='", "'", NULL);
        int cp = get_info_int(update, "clan_points='", "'", NULL);
        int cr = get_info_int(update, "clan_role='", "'", NULL);

        if (pid == NULL)
        {
            free(jid);
            free(pid);
            free(nick);
            free(update);
            ++m;
            continue;
        }

        /* If it's us */
        if (0 == strcmp(pid, session.profile.id))
        {
            session.profile.clan.points = cp;
            session.profile.clan.role = cr;
        }
        else
        {
            char *real_nick = NULL;

            if (nick == NULL)
            {
                struct clanmate *c = clanmate_list_get_by_pid(pid);

                if (c != NULL && c->nickname != NULL)
                    real_nick = strdup(c->nickname);
            }
            else
                real_nick = strdup(nick);

            if (real_nick != NULL)
            {
                enum clan_update ret =
                    clanmate_list_update(jid, real_nick, pid,
                                         status, exp, cp, cr);

                switch (ret)
                {
                    case CLAN_UPDATE_JOINED:
                    {
                        char *s = LANG_FMT(notif_clan_joined, real_nick);
                        xprintf("%s", s);
                        free(s);
                        break;
                    }
                    case CLAN_UPDATE_LEFT:
                    {
                        char *s = LANG_FMT(notif_clan_left, real_nick);
                        xprintf("%s", s);
                        free(s);
                        break;
                    }
                    default:
                        break;
                }
            }

            free(real_nick);
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
