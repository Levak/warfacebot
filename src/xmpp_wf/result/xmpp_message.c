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
#include <string.h>

static void xmpp_message_cb(const char *msg_id, const char *msg)
{
    if (strstr(msg, "type='result'"))
        return;

    char *message = get_info(msg, "message='", "'", NULL);
    char *id = get_info(msg, "id='", "'", NULL);
    char *nick_from = get_info(msg, "<message from='", "'", NULL);
    char *jid_from = get_info(msg, "<iq from='", "'", NULL);

    /* 1. Feedback the user what was sent (SERIOUSLY ? CRYTEK ? XMPP 4 DUMMIES ?) */

    xmpp_send_message(session.wfs, nick_from, session.jid,
                      session.nickname, jid_from,
                      message, id);

    /* */

    if (strstr(message, "leave"))
    {
        xmpp_iq_gameroom_leave();

        xmpp_send_message(session.wfs, session.nickname, session.jid,
                          nick_from, jid_from,
                          "but whyy :(", NULL);
    }

    else if (strstr(message, "ready"))
    {
        send_stream_format(session.wfs,
                           "<iq to='masterserver@warface/%s' type='get'>"
                           " <query xmlns='urn:cryonline:k01'>"
                           "  <gameroom_setplayer team_id='0' status='1' class_id='0'/>"
                           " </query>"
                           "</iq>",
                           session.channel);

        xmpp_send_message(session.wfs, session.nickname, session.jid,
                          nick_from, jid_from,
                          "go", NULL);
    }

    else if (strstr(message, "invite"))
    {
        send_stream_format(session.wfs,
                           "<iq to='masterserver@warface/%s' type='get'>"
                           " <query xmlns='urn:cryonline:k01'>"
                           "  <invitation_send nickname='%s' is_follow='0'/>"
                           " </query>"
                           "</iq>",
                           session.channel, nick_from);
    }

    else if (strstr(message, "master"))
    {
        xmpp_promote_room_master(nick_from);

        xmpp_send_message(session.wfs, session.nickname, session.jid,
                          nick_from, jid_from,
                          "Yep, just a sec.", NULL);

    }

    else
    {
        /* Command not found */
        xmpp_send_message(session.wfs, session.nickname, session.jid,
                          nick_from, jid_from,
                          "I&apos;m sorry Dave. I&apos;m afraid I can&apos;t do that.", NULL);
    }

    free(id);
    free(jid_from);
    free(nick_from);
    free(message);
}

void xmpp_message_r(void)
{
    qh_register("message", xmpp_message_cb);
}
