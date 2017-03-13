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
#include <wb_list.h>
#include <wb_log.h>
#include <wb_lang.h>

#include <stdlib.h>
#include <string.h>

#define GAMEROOM_GET_BATCH_SIZE 100

struct cb_args
{
    struct list *room_list;
    enum room_type type;
    char *channel;

    f_gameroom_get_cb cb;
    void *args;
};

static int _gameroom_cmp(struct gameroom *gr1, struct gameroom *gr2)
{
    return strcmp(gr1->room_id, gr2->room_id);
}

static void _gameroom_free(struct gameroom *gr)
{
    gameroom_free(gr);
    free(gr);
}

static void xmpp_iq_gameroom_get_cb(const char *msg,
                                          enum xmpp_msg_type type,
                                          void *args)
{
    /* Answer :
       <iq to='masterserver@warface/pve_2' type='get'>
        <query xmlns='urn:cryonline:k01'>
         <gameroom_get left='xx' token='xx'>
          <game_room room_id='...' room_type='16'>
           <core room_name='...' private='0' players='4' .../>
           <mission mission_key='...' no_teams='1' ...> ... </mission>
           <session status='2' game_progress='1' .../>
           ...
           <room_master master='xxx'/>
           <clan_war clan_1='xxxx' clan_2='xxxxxx'/>
          </game_room>
          ...
         </gameroom_get>
        </query>
       </iq>
     */

    struct cb_args *a = (struct cb_args *) args;

    if (type & XMPP_TYPE_ERROR)
    {
        const char *reason = NULL;

        int code = get_info_int(msg, "code='", "'", NULL);
        int custom_code = get_info_int(msg, "custom_code='", "'", NULL);

        switch (code)
        {
            case 8:
                switch (custom_code)
                {
                    default:
                        break;
                }
                break;
            default:
                break;
        }

        if (reason != NULL)
            eprintf("%s (%s)",
                    LANG(error_gameroom_get),
                    reason);
        else
            eprintf("%s (%i:%i)",
                    LANG(error_gameroom_get),
                    code,
                    custom_code);

        list_free(a->room_list);
        free(a->channel);
        free(a);
    }
    else
    {
        char *data = wf_get_query_content(msg);

        if (data != NULL)
        {
            const char *m = data;
            while ((m = strstr(m, "<game_room ")) != NULL)
            {
                struct gameroom *gr = calloc(1, sizeof (struct gameroom));
                char *grs = get_info(m, "<game_room", "</game_room>", NULL);

                gameroom_init(gr);
                gameroom_parse(gr, grs);
                list_add(a->room_list, gr);

                free(grs);
                ++m;
            }

            int left = get_info_int(data, "left='", "'", NULL);

            if (left > 0)
            {
                int token = get_info_int(data, "token='", "'", NULL);

                xmpp_send_iq_get(
                    JID_MS(a->channel),
                    xmpp_iq_gameroom_get_cb, a,
                    "<query xmlns='urn:cryonline:k01'>"
                    "<gameroom_get token='%d' received='%u'"
                    "              size='%u' room_type='%u'/>"
                    "</query>",
                    token,
                    a->room_list->length,
                    GAMEROOM_GET_BATCH_SIZE,
                    a->type);
            }
            else
            {
                if (a->cb)
                    a->cb(a->room_list, a->args);
                else
                    list_free(a->room_list);

                free(a->channel);
                free(a);
            }
        }
        else
        {
            list_free(a->room_list);
            free(a->channel);
            free(a);
        }

        free(data);
    }
}

void xmpp_iq_gameroom_get(const char *channel,
                          enum room_type type,
                          f_gameroom_get_cb cb,
                          void *args)
{
    if (channel == NULL)
        return;

    struct cb_args *a = calloc(1, sizeof (struct cb_args));

    a->cb = cb;
    a->args = args;
    a->type = type;
    a->channel = strdup(channel);
    a->room_list = list_new((f_list_cmp) _gameroom_cmp,
                            (f_list_free) _gameroom_free);

    xmpp_send_iq_get(
        JID_MS(a->channel),
        xmpp_iq_gameroom_get_cb, a,
        "<query xmlns='urn:cryonline:k01'>"
        " <gameroom_get token='0' received='0'"
        "               size='%u' room_type='%u'/>"
        "</query>",
        GAMEROOM_GET_BATCH_SIZE,
        type);
}

