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
#include <wb_threads.h>
#include <wb_session.h>
#include <wb_xmpp.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

static char *jid_to_str(const struct jid *jid)
{
    char *jid_s;

    assert(jid->domain != NULL);

    jid_s = strdup(jid->domain);

    if (jid->node != NULL)
    {
        char *s;
        FORMAT(s,
               "%s@%s",
               jid->node,
               jid_s);
        free(jid_s);
        jid_s = s;
    }

    if (jid->resource != NULL)
    {
        char *s;
        FORMAT(s,
               "%s/%s",
               jid_s,
               jid->resource);
        free(jid_s);
        jid_s = s;
    }

    return jid_s;
}

static inline char *vformat(const char *fmt, va_list ap)
{
    size_t len;
    char *s;
    va_list ap2;

    va_copy(ap2, ap);

    len = vsnprintf(NULL, 0, fmt, ap);

    s = malloc(len + 1);

    vsnprintf(s, len + 1, fmt, ap2);

    va_end(ap2);

    return s;
}

void xmpp_send(const char *fmt,
               ...)
{
    va_list ap;
    char *query;

    va_start(ap, fmt);
    query = vformat(fmt, ap);
    va_end(ap);

    thread_sendstream_post_new_msg(query);
}

static char *iq_create(const char *id,
                       const struct jid *target,
                       enum xmpp_msg_type type,
                       const char *query)
{
    const char *type_s;
    {
        switch (type)
        {
            case XMPP_TYPE_RESULT:
                type_s = "result";
                break;
            case XMPP_TYPE_ERROR:
                type_s = "error";
                break;
            case XMPP_TYPE_SET:
                type_s = "set";
                break;
            case XMPP_TYPE_GET:
            default:
                type_s = "get";
        }
    }

    char *target_s = NULL;

    if (target != NULL)
        target_s = jid_to_str(target);

    const char *from_s = session.xmpp.jid;

    char *iq;
    FORMAT(iq,
           "<iq xmlns='jabber:client'"
           " %s%s%s %s%s%s %s%s%s type='%s' %s%s%s",
           target_s ? "to='" : "",
           target_s ? target_s : "",
           target_s ? "'" : "",
           from_s ? "from='" : "",
           from_s ? from_s : "",
           from_s ? "'" : "",
           id ? "id='" : "",
           id ? id : "",
           id ? "'" : "",
           type_s,
           query ? ">" : "/>",
           query ? query : "",
           query ? "</iq>" : "");

    free(target_s);

    return iq;
}

void xmpp_send_iq(const struct jid *target,
                  enum xmpp_msg_type type,
                  f_id_callback cb,
                  void *args,
                  const char *fmt,
                  ...)
{
    va_list ap;
    char *query;
    char *iq;

    char *id;
    t_uid id_;

    if (cb != NULL && (type & (XMPP_TYPE_GET | XMPP_TYPE_SET)))
    {
        idh_generate_unique_id(&id_);
        idh_register(&id_, 0, cb, args);
        id = (char *) &id_;
    }
    else
    {
        id = (char *) args;
    }

    if (fmt != NULL)
    {
        va_start(ap, fmt);
        query = vformat(fmt, ap);
        va_end(ap);
    }
    else
        query = NULL;

    iq = iq_create(id, target, type, query);

    free(query);

    thread_sendstream_post_new_msg(iq);
}
