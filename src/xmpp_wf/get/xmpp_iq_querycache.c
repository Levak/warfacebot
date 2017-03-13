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
#include <wb_item.h>
#include <wb_querycache.h>
#include <wb_cvar.h>
#include <wb_log.h>

struct cb_args
{
    struct querycache *cache;
    f_querycache_cb cb;
    void *args;
};

static void querycache_request_cb(const char *msg,
                                  enum xmpp_msg_type type,
                                  void *args)
{
    /* Answer :
       <iq to='xxx@warface/GameClient' type='result'>
        <query xmlns='urn:cryonline:k01'>
         <items code='2' from='0' to='250' hash='271893941'>
          <item id='xxxxx' name='xxxx' locked='0' />
          ...
        </query>
       </iq>
     */

    struct cb_args *a = (struct cb_args *) args;

    if (type & XMPP_TYPE_ERROR || msg == NULL)
    {
        free(a);
        return;
    }

    char *data = wf_get_query_content(msg);

    if (data == NULL)
    {
        free(a);
        return;
    }

    char *hash = get_info(data, "hash='", "'", NULL);
    int code = get_info_int(data, "code='", "'", NULL);
    int from = get_info_int(data, "from='", "'", NULL);
    int to = get_info_int(data, "to='", "'", NULL);

    switch (code)
    {
        case XMPP_CHUNK_CACHED:
#ifdef DEBUG
            xprintf("Cache for `%s' is up to date", a->cache->queryname);
#endif /* DEBUG */
            if (a->cb != NULL)
                a->cb(a->cache, args);

            free(a);
            break;

        case XMPP_CHUNK_MORE:
            querycache_update(a->cache, data, hash, from, to, 0);

            from = to;
            xmpp_send_iq_get(
                JID_ANY_MS,
                querycache_request_cb, a,
                "<query xmlns='urn:cryonline:k01'>"
                " <%s from='%d'/>"
                "</query>",
                a->cache->queryname,
                from);
            break;

        case XMPP_CHUNK_END:
            querycache_update(a->cache, data, hash, from, to, 1);

            if (a->cb != NULL)
                a->cb(a->cache, args);

            free(a);
            break;

        default:
            free(a);
            break;
    }

    free(hash);
    free(data);
}

void querycache_request(struct querycache *cache,
                        enum querycache_mode mode,
                        f_querycache_cb cb,
                        void *args)
{
    if (cache == NULL || cache->queryname == NULL)
        return;

    struct cb_args *a = calloc(1, sizeof (struct cb_args));

    a->cb = cb;
    a->args = args;
    a->cache = cache;

    const char *hash = cvar.query_cache ? cache->hash : 0;

    if (mode == QUERYCACHE_ANY_CHANNEL)
    {
        xmpp_send_iq_get(
            JID_ANY_MS,
            querycache_request_cb, a,
            "<query xmlns='urn:cryonline:k01'>"
            " <%s cached='%s'/>"
            "</query>",
            cache->queryname,
            hash);
    }
    else
    {
        xmpp_send_iq_get(
            JID_MS(session.online.channel),
            querycache_request_cb, a,
            "<query xmlns='urn:cryonline:k01'>"
            " <%s cached='%s'/>"
            "</query>",
            cache->queryname,
            hash);
    }
}
