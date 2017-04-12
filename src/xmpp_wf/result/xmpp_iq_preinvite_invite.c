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
#include <wb_log.h>
#include <wb_cvar.h>
#include <wb_lang.h>

#include <stdlib.h>

static void xmpp_iq_preinvite_invite_cb(const char *msg_id,
                                        const char *msg,
                                        void *args)
{
    /* Accept any preinvite
       <iq from='xxxx@warface/GameClient' id='uid000000e9' type='get'>
        <query xmlns='urn:cryonline:k01'>
         <preinvite_invite from='xxxx' uid='xxxx'
                           ms_resource='xxx' channel_type='pve'
                           mission_id='xxxx'
                           clan_name='xxx' experience='xxxxxx'
                           badge='xxx' mark='xxx' stripe='xxx'
                           onlineId='xxx' profileId='xxx' is_online='1'/>
        </query>
       </iq>
     */

    char *data = wf_get_query_content(msg);

    if (!data)
        return;

    char *jid = get_info(msg, "from='", "'", NULL);

    char *uid = get_info(data, "uid='", "'", NULL);
    char *from = get_info(data, "from='", "'", NULL);
    char *ms_resource = get_info(data, "ms_resource='", "'", NULL);
    char *channel_type = get_info(data, "channel_type='", "'", NULL);
    char *mission_id = get_info(data, "mission_id='", "'", NULL);

    if (jid != NULL && uid != NULL && ms_resource != NULL
        && channel_type != NULL && mission_id != NULL)
    {
        char accepted = !cvar.wb_safemaster
            && cvar.wb_accept_room_invitations
            && session.quickplay.pre_uid == NULL
            && session.quickplay.uid == NULL;

        char postponed = cvar.wb_postpone_room_invitations;

        {
            char *s = LANG_FMT(notif_room_preinvitation, from);
            xprintf("%s (%s)",
                    s,
                    (postponed)
                    ? LANG(notif_postponed)
                    : (accepted)
                    ? LANG(notif_accepted)
                    : LANG(notif_rejected));
            free(s);
        }

        if (!postponed)
        {
            if (accepted)
            {
                free(session.quickplay.pre_uid);
                session.quickplay.pre_uid = strdup(uid);

                xmpp_iq_gameroom_leave();
            }

            xmpp_send_iq_result(
                JID(jid),
                msg_id,
                "<query xmlns='urn:cryonline:k01'>"
                " <preinvite_invite uid='%s' accepted='%d' from='%s'"
                "    mission_id='%s' channel_type='%s' ms_resource='%s'/>"
                "</query>",
                uid,
                accepted,
                from,
                mission_id,
                channel_type,
                ms_resource);

            xmpp_send_iq_get(
                JID(jid),
                NULL, NULL,
                "<query xmlns='urn:cryonline:k01'>"
                " <preinvite_response uid='%s' accepted='%d'"
                "    pid='%s' from='%s'/>"
                "</query>",
                uid,
                accepted,
                session.profile.id,
                session.profile.nickname);
        }
    }

    free(uid);
    free(from);
    free(jid);
    free(mission_id);
    free(channel_type);
    free(ms_resource);
    free(data);
}

void xmpp_iq_preinvite_invite_r(void)
{
    qh_register("preinvite_invite", 1, xmpp_iq_preinvite_invite_cb, NULL);
}
