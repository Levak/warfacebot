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
#include <wb_session.h>
#include <wb_xmpp.h>
#include <wb_xmpp_wf.h>

#include <stdlib.h>
#include <string.h>

struct cb_args
{
    f_id_callback cb;
    void *args;
};

static void xmpp_iq_get_achievements_cb(const char *msg,
                                        enum xmpp_msg_type type,
                                        void *args)
{
    /* Answer:
       <iq from='masterserver@warface/xx' type='result'>
        <query xmlns='urn:cryonline:k01'>
         <get_achievements>
          <achievement profile_id="1597755">
           <chunk achievement_id="51" progress="100000" completion_time="1461010526"/>
           <chunk achievement_id="52" progress="522318" completion_time="0"/>
           <chunk achievement_id="53" progress="522318" completion_time="0"/>
           <chunk achievement_id="54" progress="10" completion_time="1459026091"/>
          </achievement>
          ...
         </get_achievements>
        </query>
       </iq>
    */

    struct cb_args *a = (struct cb_args *) args;

    if (type ^ XMPP_TYPE_ERROR)
    {
        char *data = wf_get_query_content(msg);

        if (data != NULL)
        {
            /* Fetch chunks */
            {
                unsigned int achievement_count = 0;

                const char *m = data;

                while ((m = strstr(m, "<chunk ")))
                {
                    char *chunk = get_info(m, "<chunk ", "/>", NULL);
                    unsigned int completion_time =
                        get_info_int(chunk, "completion_time='", "'", NULL);

                    if (completion_time > 0)
                        ++achievement_count;

                    free(chunk);
                    ++m;
                }

                if (achievement_count > 0)
                    session.profile.stats.challenges_completed = achievement_count;
            }
        }

        free(data);
    }

    if (a->cb)
        a->cb(msg, type, a->args);

    free(a);
}

void xmpp_iq_get_achievements(const char *profile_id,
                              f_id_callback cb,
                              void *args)
{
    struct cb_args *a = calloc(1, sizeof (struct cb_args));

    a->cb = cb;
    a->args = args;

    xmpp_send_iq_get(
        JID_MS(session.online.channel),
        xmpp_iq_get_achievements_cb, a,
        "<query xmlns='urn:cryonline:k01'>"
        "<get_achievements>"
        "<achievement profile_id='%s'/>"
        "</get_achievements>"
        "</query>",
        profile_id);
}
