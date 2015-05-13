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

#if defined(_WIN32) || defined(__CYGWIN__)
# define MSG_MORE 0x8000
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <strings.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>

#ifdef DEBUG
# include <readline/readline.h>
# include <readline/history.h>
#endif

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#ifndef ZLIB
# define ZLIB "zlib"
#endif

#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/comp.h>

#define GAME_VERSION "1.1.1.3570"
//#define STAT_BOT
#define XMPP_ID "uid%08d"

/** STREAM **/

struct stream_hdr
{
    uint32_t magic;

    uint32_t len;

    /**
     * 0 = No XOR
     * 1 = XOR
     * 2 = send UID
     * 3 = XOR key
     * 4 = ACK
     */
    uint32_t xor;
};

void send_stream(int fd, char *msg, uint32_t msg_size)
{
    ssize_t wrote_size = 0;
    struct stream_hdr hdr;

    hdr.magic = 0xFEEDDEAD;
    hdr.xor = 0;
    hdr.len = msg_size;

    send(fd, &hdr, sizeof (hdr), MSG_MORE);
    wrote_size = send(fd, msg, msg_size, MSG_MORE);

#ifdef DEBUG
    printf("--(%3u/%3u)-> ", wrote_size, msg_size);
    printf("\033[1;31m%s\033[0m\n", msg);
#endif
}

void send_stream_ascii(int fd, char *msg)
{
    send_stream(fd, msg, strlen(msg));
}

void flush_stream(int fd)
{
    send(fd, "", 0, 0);
}

#define FORMAT_STRING_SIZE 255
void send_stream_format(int fd, char *fmt, ...)
{
    unsigned int len;
    va_list ap;
    char s[FORMAT_STRING_SIZE];

    va_start(ap, fmt);
    len = vsnprintf(s, FORMAT_STRING_SIZE, fmt, ap);
    va_end(ap);

    if (len >= FORMAT_STRING_SIZE)
    {
        char *s2 = malloc(len + 1);

        va_start(ap, fmt);
        vsprintf(s2, fmt, ap);
        va_end(ap);

        send_stream(fd, s2, len);
        free(s2);
    }
    else
        send_stream(fd, s, len);

    flush_stream(fd);
}

char *read_stream_keep(int fd)
{
    char *msg;
    struct stream_hdr hdr;

    if (read(fd, &hdr, sizeof (hdr)) != sizeof (hdr))
        return NULL;

    if (hdr.magic != 0xFEEDDEAD)
    {
        printf("============== BAD MAGIC NUMBER ===============");
        return NULL;
    }

    msg = calloc(hdr.len + 1, 1);
    char *curr_pos = msg;
    ssize_t read_size = 0;
    ssize_t size = 0;

    do {
        size = read(fd, curr_pos, hdr.len - (curr_pos - msg));
        read_size += size;
        curr_pos += size;
    } while (read_size < hdr.len && size > 0);

#ifdef DEBUG
    printf("<-(%3u/%3u)-- ", read_size, hdr.len);
    printf("\033[1;32m%s\033[0m\n", msg);
#endif

    return msg;
}

int read_stream(int fd)
{
    char *msg = read_stream_keep(fd);
    int size = strlen(msg);

    free(msg);
    return size;
}

int connect_wf(char *hostname, int port)
{
    int wfs = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serv_addr;
    struct hostent *server;

    server = gethostbyname(hostname);
    if (server == NULL)
        fprintf(stderr, "ERROR gethostbyname\n");

    bzero((char *) &serv_addr, sizeof(serv_addr));
    bcopy((char *) server->h_addr,
          (char *) &serv_addr.sin_addr.s_addr,
          server->h_length);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (connect(wfs, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        fprintf(stderr, "ERROR connect\n");
        fprintf(stderr, "%s\n", strerror(errno));
    }


    return wfs;
}

/** QUERY HANDLERS **/

typedef struct { char uid[12]; } t_uid;
typedef void (*f_query_callback)(const char *msg);

struct query_handler {
    f_query_callback callback;
    char permanent;
    t_uid id;
};

#define QUERY_HDLR_MAX 16
struct query_handler query_handlers[QUERY_HDLR_MAX] = { 0 };

void register_query(const t_uid *id, f_query_callback callback, char perm)
{
    int i = 0;
    for (; i < QUERY_HDLR_MAX; ++i)
        if (!query_handlers[i].id.uid[0])
            break;

    if (!query_handlers[i].id.uid[0])
    {
        query_handlers[i].id = *id;
        query_handlers[i].callback = callback;
        query_handlers[i].permanent = perm;
    }
}

int handle_queries(const char *msg_id, const char *msg)
{
    if (!msg_id)
        return 0;

    int i = 0;
    for (; i < QUERY_HDLR_MAX; ++i)
    {
        if (strncmp(query_handlers[i].id.uid, msg_id, sizeof (query_handlers[i].id.uid)) == 0)
        {
            if (!query_handlers[i].permanent)
                query_handlers[i].id.uid[0] = 0;
            query_handlers[i].callback(msg);
            return 1;
        }
    }

    return 0;
}

void generate_unique_id(t_uid *id)
{
    static unsigned int _id = 0;
    ++_id;

    sprintf((char *)id->uid, XMPP_ID, _id);
}

/** STANZA HANDLERS **/

typedef void (*f_stanza_callback)(const char *msg_id, const char *msg);

struct stanza_handler {
    f_stanza_callback callback;
    char stanza[32];
};

#define STANZA_HDLR_MAX 16
struct stanza_handler stanza_handlers[STANZA_HDLR_MAX] = { 0 };

void register_stanza(const char *stanza, f_stanza_callback callback)
{
    int i = 0;
    for (; i < STANZA_HDLR_MAX; ++i)
        if (!stanza_handlers[i].callback)
            break;

    if (!stanza_handlers[i].callback)
    {
        stanza_handlers[i].callback = callback;
        strncpy(stanza_handlers[i].stanza, stanza, sizeof (stanza_handlers[i].stanza));
    }
}

int handle_stanza(const char *stanza, const char *msg_id, const char *msg)
{
    if (!stanza || !*stanza)
        return 0;

    int i = 0;
    for (; i < STANZA_HDLR_MAX; ++i)
    {
        if (strncmp(stanza_handlers[i].stanza, stanza, sizeof(stanza_handlers[i].stanza)) == 0)
        {
            stanza_handlers[i].callback(msg_id, msg);
            return 1;
        }
    }

    return 0;
}

/** TOOLS **/

char *get_info(const char *input, const char *patt_b, const char *patt_e, const char *desc)
{
    int size_b = strlen(patt_b);
    char *ret = NULL;
    char *start = NULL;
    char *end = NULL;

    start = strstr(input, patt_b);

    if (start)
        end = strstr(start + size_b, patt_e);

    if (start && end)
    {
        start += size_b;
        ret = calloc(end - start + 1, 1);
        strncpy(ret, start, end - start);
        ret[end - start] = 0;
        if (desc)
            printf("%s is %s\n", desc, ret);
    }
    else if (desc)
        fprintf(stderr, "Could not find %s\n", desc);

    return ret;
}

char *get_info_first(const char *input, const char *patt_b, const char *patt_e, const char *desc)
{
    int size_b = strlen(patt_b);
    char *ret = NULL;
    char *start = NULL;
    char *end = NULL;

    start = strstr(input, patt_b);

    if (start)
    {
        for (; *patt_e; ++patt_e)
        {
            char *nend = strchr(start + size_b, *patt_e);
            if (!end)
                end = nend;
            else if (nend && nend < end)
                end = nend;
        }
    }

    if (start && end)
    {
        start += size_b;
        ret = calloc(end - start + 1, 1);
        strncpy(ret, start, end - start);
        ret[end - start] = 0;
        if (desc)
            printf("%s is %s\n", desc, ret);
    }
    else if (desc)
        fprintf(stderr, "Could not find %s\n", desc);

    return ret;
}

char *get_msg_id(const char *msg)
{
    char *msg_id = NULL;
    char *first = get_info(msg, "<", ">", NULL);

    if (first)
    {
        msg_id = get_info(first, "id='", "'", NULL);
        free(first);
    }

    return msg_id;
}

char *get_query_tag_name(const char *msg)
{
    char *stanza = NULL;
    char *iq_pos = strstr(msg, "<iq");

    if (iq_pos)
    {
        iq_pos += sizeof ("<iq") - 1;
        char *query_pos = strstr(iq_pos, "<query");
        if (query_pos)
        {
            query_pos += sizeof ("<query") - 1;
            char *data_pos = strstr(query_pos, "<data");
            if (data_pos)
            {
                data_pos += sizeof ("<data") - 1;
                stanza = get_info(data_pos, "query_name='", "'", NULL);
            }
            else
                stanza = get_info_first(query_pos, "<", "/> ", NULL);
        }
        else
            stanza = get_info_first(iq_pos, "<", "/> ", NULL);
    }
    else
        stanza = get_info_first(msg, "<", "/> ", NULL);

    if (stanza && !*stanza)
    {
        free(stanza);
        stanza = NULL;
    }

    return stanza;
}

char *base64encode(const void *input, size_t inlength)
{
    BIO *bmem;
    BIO *b64;
    BUF_MEM *bptr;

    b64 = BIO_new(BIO_f_base64());
    bmem = BIO_new(BIO_s_mem());
    BIO_push(b64, bmem);

    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

    BIO_write(b64, input, inlength);
    BIO_flush(b64);
    BIO_get_mem_ptr(b64, &bptr);

    char *buff = malloc(bptr->length + 1);
    memcpy(buff, bptr->data, bptr->length);
    buff[bptr->length] = 0;

    BIO_free_all(b64);

    return buff;
}

size_t base64length(const char* b64input, size_t len)
{
    size_t padding = 0;

    if (b64input[len-1] == '=' && b64input[len-2] == '=')
        padding = 2;
    else if (b64input[len-1] == '=')
        padding = 1;

    return ((len * 3) >> 2) - padding;
}

char *base64decode(const void *input, size_t inlength, size_t *outlength)
{
    BIO *bmem;
    BIO *b64;
    BUF_MEM *bptr;

    *outlength = base64length(input, inlength);
    char *buff = malloc(*outlength + 1);

    b64 = BIO_new(BIO_f_base64());
    bmem = BIO_new_mem_buf((void *) input, inlength);
    BIO_push(b64, bmem);

    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

    BIO_read(b64, buff, inlength);

    BIO_free_all(b64);

    return buff;
}

char *zlibb64encode(const void *input, size_t inlength)
{ /* Untested */
    BIO *bmem;
    BIO *bz;
    BIO *b64;
    BUF_MEM *bptr;

    bz = BIO_new(BIO_f_zlib());
    b64 = BIO_new(BIO_f_base64());
    bmem = BIO_new(BIO_s_mem());
    BIO_push(bz, b64);
    BIO_push(b64, bmem);

    BIO_write(bz, input, inlength);
    BIO_flush(bz);
    BIO_get_mem_ptr(bz, &bptr);

    char *buff = malloc(bptr->length);
    memcpy(buff, bptr->data, bptr->length);

    BIO_free_all(bz);

    return buff;
}

char *zlibb64decode(const void *input, size_t inlength, size_t outlength)
{
    BIO *bmem;
    BIO *bz;
    BIO *b64;
    BIO *buf_io;
    char *buffer = calloc(outlength + 1, 1);

    bz = BIO_new(BIO_f_zlib());
    b64 = BIO_new(BIO_f_base64());
    bmem = BIO_new_mem_buf((void *) input, inlength);

    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

    BIO_push(bz, b64);
    BIO_push(b64, bmem);

    BIO_read(bz, buffer, outlength);
    buffer[outlength] = 0;

    BIO_free_all(bz);

    return buffer;
}

/** XMPP TOOLS **/

char *combine_logins(const char *login, const char *mdp)
{
    int lsize = strlen(login);
    int msize = strlen(mdp);
    int size = lsize + msize + 2;
    char *logins = calloc(size + 1, 1);

    strcpy(logins + 1, login);
    strcpy(logins + lsize + 2, mdp);

    char *encoded = base64encode(logins, size);

    free(logins);
    return encoded;
}

void print_number_of_occupants_cb(const char *msg)
{
    char *num = get_info(msg, "var='muc#roominfo_occupants'><value>", "</value>", NULL);

    fprintf(stderr, "%u %s\n", time(NULL), num);

    free(num);
}

void print_number_of_occupants(int wfs, char *room)
{
    t_uid id;

    generate_unique_id(&id);
    register_query(&id, &print_number_of_occupants_cb, 0);

    send_stream_format(wfs,
                       "<iq to='%s' type='get' id='%s'>"
                       " <query xmlns='http://jabber.org/protocol/disco#info'/>"
                       "</iq>",
                       room, &id);
}

void print_number_of_players_cb(const char *msg)
{
    unsigned int count_all = 0;
    unsigned int count_pvp = 0;
    unsigned int count_pve = 0;

    const char *m = msg;
    while ((m = strstr(m, "<item")))
    {
        /* Extract room jid */
        char *rjid = get_info(m, "jid='", "'", NULL);

        /* It's not a clan room */
        if (!strstr(rjid, "clan"))
        {
            char *c = get_info(m, "(", ")", NULL);
            unsigned int count = strtol(c, NULL, 10);
            free(c);

            if (strstr(rjid, "pve"))
                count_pve += count;
            else if (strstr(rjid, "pvp"))
                count_pvp += count;
            count_all += count;
        }

        free(rjid);
        m += 5;
    }

    printf("%u,%u,%u,%u\n", time(NULL), count_all, count_pve, count_pvp);
    fflush(stdout);
}

char *decode_compressed_data(const char *msg)
{
    char *compressedData = strstr(msg, "compressedData='");

    if (!compressedData)
        return get_info(msg, "urn:cryonline:k01'>", "</query>", NULL);

    compressedData += sizeof ("compressedData='") - 1;

    char *originalSize = strstr(msg, "originalSize='");

    if (!originalSize)
        return NULL;

    originalSize += sizeof ("originalSize='") - 1;

    size_t outsize = strtol(originalSize, NULL, 10);
    size_t insize = strstr(compressedData, "'") - compressedData + 1;

    return zlibb64decode(compressedData, insize, outsize);
}

int xmpp_is_error(const char *msg)
{
    return strstr(msg, "<error") != NULL;
}

/** WARFACE XMPP TOOLS **/

void xmpp_send_message(int wfs,
                       const char *from_login, const char *from_jid,
                       const char *to_login, const char *to_jid,
                       const char *msg, const char *answer_id)
{
    const char *mid = NULL;
    t_uid id;

    if (answer_id)
        mid = answer_id;
    else
    {
        generate_unique_id(&id);
        mid = (char *) &id;
    }

    send_stream_format(wfs,
                       "<iq from='%s' to='%s' type='%s' id='%s'>"
                       "<query xmlns='urn:cryonline:k01'>"
                       "<message from='%s' nick='%s' message='%s'/>"
                       "</query>"
                       "</iq>",
                       from_jid, to_jid,
                       answer_id ? "result" : "get",
                       mid,
                       from_login, to_login, msg);
}

/** WARFACE SESSION **/

enum e_status
{
    STATUS_OFFLINE   = 0,
    STATUS_ONLINE    = 1 << 0,
    STATUS_UNK       = 1 << 1,
    STATUS_AFK       = 1 << 2,
    STATUS_LOBBY     = 1 << 3,
    STATUS_ROOM      = 1 << 4,
    STATUS_PLAYING   = 1 << 5,
    STATUS_SHOP      = 1 << 6,
    STATUS_INVENTORY = 1 << 7,
};

static struct
{
    int wfs;
    char active;
    char *jid;
    char *nickname;
    char *active_token;
    char *profile_id;
    char *online_id;
    char *channel;
    int status;
    char *friend; /* TODO: List */
    char *group_id;
} session = { 0 };

/** XMPP QUERY HANDLERS **/

void xmpp_bind(const char *resource);
void xmpp_iq_session(void);
void xmpp_iq_account(void);
void xmpp_iq_get_account_profiles(void);
void xmpp_iq_join_channel(const char *channel);
void xmpp_iq_player_status(int status);
void xmpp_iq_peer_status_update(const char *to_jid);

void xmpp_bind_cb(const char *msg)
{
    /* Answer :
      <iq id='bind_1' type='result'>
         <bind xmlns='urn:ietf:params:xml:ns:xmpp-bind'>
           <jid>XXXX@warface.com/GameClient</jid>
         </bind>
       </iq>
    */

    session.jid = get_info(msg, "<jid>", "</jid>", "JID");

    xmpp_iq_session();
}

void xmpp_bind(const char *resource)
{
    t_uid id;

    generate_unique_id(&id);
    register_query(&id, xmpp_bind_cb, 0);

    /* Bind stream and get JID */
    send_stream_format(session.wfs,
                       "<iq id='%s' type='set'>"
                       "  <bind xmlns='urn:ietf:params:xml:ns:xmpp-bind'>"
                       "    <resource>%s</resource>"
                       "  </bind>"
                       "</iq>",
                       &id, resource);
}

void xmpp_iq_session_cb(const char *msg)
{
    xmpp_iq_account();
}

void xmpp_iq_session(void)
{
    t_uid id;

    generate_unique_id(&id);
    register_query(&id, xmpp_iq_session_cb, 0);

    /* Bind the session */
    send_stream_format(session.wfs,
                       "<iq id='%s' from='%s' type='set' xmlns='jabber:client'>"
                       "  <session xmlns='urn:ietf:params:xml:ns:xmpp-session'/>"
                       "</iq>",
                       &id, session.jid);
}

void xmpp_iq_account_cb(const char *msg)
{
    /* Answer :
       <iq from='k01.warface' to='XXXX@warface/GameClient' type='result'>
         <query xmlns='urn:cryonline:k01'>
           <account user='XXXX' active_token='$WF_XXXX_....'
            load_balancing_type='server'>
             <masterservers>
               <server .../>
               ...
     */

    if (xmpp_is_error(msg))
    {
        return;
    }

    session.status = STATUS_ONLINE;
    session.active_token = get_info(msg, "active_token='", "'", "ACTIVE TOKEN");

    xmpp_iq_get_account_profiles();
}

void xmpp_iq_account(void)
{
    t_uid id;

    generate_unique_id(&id);
    register_query(&id, xmpp_iq_account_cb, 0);

    send_stream_format(session.wfs,
                       "<iq id='%s' to='k01.warface' type='get'>"
                       "<query xmlns='urn:cryonline:k01'>"
                       "<account login='%s'/>" /* Don't put any space there ! */
                       "</query>"
                       "</iq>",
                       &id, session.online_id);
}

void xmpp_iq_get_account_profiles_cb(const char *msg)
{
    /* Answer :
       <iq from="masterserver@warface/pve_12" type="result">
         <query xmlns="urn:cryonline:k01">
           <get_account_profiles>
             <profile id="XXX" nickname="XXX"/>
           </get_account_profiles>
         </query>
       </iq>
    */

    session.profile_id = get_info(msg, "profile id='", "'", "PROFILE ID");
    session.nickname = get_info(msg, "nickname='", "'", "NICKNAME");

    xmpp_iq_join_channel("pve_12");
}

void xmpp_iq_get_account_profiles(void)
{
    t_uid id;

    generate_unique_id(&id);
    register_query(&id, xmpp_iq_get_account_profiles_cb, 0);

    /* Get CryOnline profile */
    send_stream_format(session.wfs,
                       "<iq id='%s' to='ms.warface' type='get'>"
                       " <query xmlns='urn:cryonline:k01'>"
                       "  <get_account_profiles version='" GAME_VERSION "'"
                       "    user_id='%s' token='%s'/>"
                       " </query>"
                       "</iq>",
                       &id,
                       session.online_id, session.active_token);
}

void xmpp_iq_join_channel_cb(const char *msg)
{
    /* Answer
      <iq from='masterserver@warface/pve_12' to='xxxxxx@warface/GameClient' type='result'>
       <query xmlns='urn:cryonline:k01'>
        <data query_name='join_channel' compressedData='...' originalSize='13480'/>
       </query>
      </iq>
     */

#if 0
    char *data = decode_compressed_data(msg);

    printf("\n\nDECODED:\n%s\n\n", data);

    free(data);
#endif

    /* Inform to k01 our status */
    xmpp_iq_player_status(STATUS_ONLINE | STATUS_LOBBY);
}

void xmpp_iq_join_channel(const char *channel)
{
    t_uid id;

    generate_unique_id(&id);
    register_query(&id, xmpp_iq_join_channel_cb, 0);

    session.channel = strdup(channel);

    /* Join CryOnline channel */
    send_stream_format(session.wfs,
                       "<iq id='%s' to='k01.warface' type='get'>"
                       "<query xmlns='urn:cryonline:k01'>"
                       "<join_channel version='" GAME_VERSION "' token='%s'"
                       "     profile_id='%s' user_id='%s' resource='%s'"
                       "     user_data='' hw_id='' build_type='--release'/>"
                       "</query>"
                       "</iq>",
                       &id,
                       session.active_token, session.profile_id,
                       session.online_id, session.channel);
}

void xmpp_iq_peer_status_update(const char *to_jid)
{
    /* Inform to our friends our status */
    send_stream_format(session.wfs,
                       "<iq to='%s' type='get'>"
                       " <query xmlns='urn:cryonline:k01'>"
                       "  <peer_status_update nickname='%s' profile_id='%s'"
                       "     status='%d' experience='0'"
                       "     place_token='' place_info_token=''/>"
                       " </query>"
                       "</iq>",
                       to_jid,
                       session.nickname, session.profile_id, session.status);
}

void xmpp_promote_room_master_cb(const char *msg)
{
    /* Answer
       <iq from='k01.warface' to='19997306@warface/GameClient' type='result'>
        <query xmlns='urn:cryonline:k01'>
         <profile_info_get_status nickname='xxxxxx'>
          <profile_info>
           <info nickname='xxxxx' online_id='xxxxx@warface/GameClient'
                 status='13' profile_id='xxx' user_id='xxxxxx' rank='xx'
                 tags='' login_time='xxxxxxxxxxx'/>
          </profile_info>
         </profile_info_get_status>
        </query>
       </iq>
     */

    char *profile_id = get_info(msg, "profile_id='", "'", "PROFILE ID");
    send_stream_format(session.wfs,
                       "<iq to='masterserver@warface/%s' type='get'>"
                       " <query xmlns='urn:cryonline:k01'>"
                       "  <gameroom_promote_to_host new_host_profile_id='%s'/>"
                       " </query>"
                       "</iq>",
                       session.channel, profile_id);

}

void xmpp_promote_room_master(const char *nickname)
{
    t_uid id;

    generate_unique_id(&id);
    register_query(&id, xmpp_promote_room_master_cb, 0);

    /* Ask server the account details of someone */
    send_stream_format(session.wfs,
                       "<iq to='k01.warface' id='%s' type='get'>"
                       "<query xmlns='urn:cryonline:k01'>"
                       "<profile_info_get_status nickname='%s'/>"
                       "</query>"
                       "</iq>",
                       &id, nickname);
}

void xmpp_iq_player_status(int status)
{
    send_stream_format(session.wfs,
                       "<iq to='k01.warface' type='get'>"
                       "<query xmlns='urn:cryonline:k01'>"
                       "<player_status prev_status='%d' new_status='%d' to='%s'/>"
                       "</query>"
                       "</iq>",
                       session.status, status, "");
    session.status = status;

    /* TODO: to friendlist */
    if (session.friend)
        xmpp_iq_peer_status_update(session.friend);

}

void xmpp_iq_gameroom_leave(void)
{
    send_stream_format(session.wfs,
                       "<iq to='masterserver@warface/%s' type='get'>"
                       " <query xmlns='urn:cryonline:k01'>"
                       "  <gameroom_leave/>"
                       " </query>"
                       "</iq>",
                       session.channel);
    xmpp_iq_player_status(STATUS_ONLINE | STATUS_LOBBY);
}

/** XMPP STANZA HANDLERS **/

void xmpp_iq_friend_list_cb(const char *msg_id, const char *msg)
{
    /* Record firends to list
       <iq from='masterserver@warface/pve_12' type='get'>
        <query xmlns='urn:cryonline:k01'>
         <friend_list>
          <friend jid='XXX' profile_id='XXX' nickname='XXX'
               status='XXX' experience='XXX' location='XXX'/>
         </friend_list>
        </query>
       </iq>
    */

    char *data = decode_compressed_data(msg);

    /* TODO: For entire friendlist */
    char *jid = get_info(data, "jid='", "'", "FRIEND JID");

#if 0
    printf("\n\nDECODED:\n%s\n\n", data);
#endif

    if (jid && *jid)
    {
        session.friend = jid;
        xmpp_iq_peer_status_update(jid);
    }
    else
        free(jid);

    free(data);
}

void xmpp_iq_peer_status_update_cb(const char *msg_id, const char *msg)
{
    /* Answer
       <iq from='xxxxxx@warface/GameClient' to='xxxxx@warface/GameClient' type='get'>
        <query xmlns='urn:cryonline:k01'>
         <peer_status_update nickname='xxxx' profile_id='xxxx' status='13'
                             experience='xxxx' place_token='@ui_playerinfo_inlobby'
                             place_info_token=''/>
        </query>
       </iq>
    */

    /* TODO: List */
    if (session.friend == NULL)
    {
        session.friend = get_info(msg, "from='", "'", "FRIEND JID");
    }
}

void xmpp_iq_ping_cb(const char *msg_id, const char *msg)
{
    /* Anwser to pings
       <iq from='warface' id='3160085767' to='XXXX@warface/GameClient' type='get'>
        <ping xmlns='urn:xmpp:ping'/>
       </iq>
    */

    send_stream_format(session.wfs,
                       "<iq from='%s' to='warface' id='%s' type='result'/>",
                       session.jid, msg_id);
}

void xmpp_message_cb(const char *msg_id, const char *msg)
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

void xmpp_iq_gameroom_sync_cb(const char *msg_id, const char *msg)
{
    char *data = decode_compressed_data(msg);
    char *game_progress = get_info(data, "game_progress='", "'", NULL);

    /* If the room has started, leave ! */
    if (game_progress != NULL && strtoll(game_progress, 0, 10) > 0)
    {
        xmpp_iq_gameroom_leave();
    }

    free(game_progress);
    free(data);
}

void xmpp_iq_invitation_request_cb(const char *msg_id, const char *msg)
{
    /* Accept any invitation
       <iq from='masterserver@warface/pve_12' id='uid0002d87c' type='get'>
        <query xmlns='urn:cryonline:k01'>
         <data query_name='invitation_request' from='XXX'
             ticket='XXXX_XXXX_XXXX' room_id='2416'
             ms_resource='pve_12' is_follow='0'
             group_id='be4ab6d9-b03a-4c2f-bd64-d8acc7e7d319'
             compressedData='...' originalSize='2082'/>
        </query>
       </iq>
     */

    char *server = get_info(msg, "from='", "'", "Server");
    char *resource = get_info(msg, "ms_resource='", "'", "Resource");
    char *ticket = get_info(msg, "ticket='", "'", "Ticket");
    char *room = get_info(msg, "room_id='", "'", "Room ID");
    char *group = get_info(msg, "group_id='", "'", "Group ID");

    if (server && resource && ticket && room && group)
    {
        /* 1. Change channel if invitation was not on the same server */
        if (strcmp(session.channel, resource))
        {
            send_stream_format(session.wfs,
                               "<iq to='k01.warface' id='switch_1' type='get'>"
                               "<query xmlns='urn:cryonline:k01'>"
                               "<switch_channel "
                               "      version='" GAME_VERSION "'"
                               "      token='%s' profile_id='%s'"
                               "      user_id='%s' resource='%s'"
                               "      user_data='' hw_id=''"
                               "      build_type='--release'/>"
                               "</query>"
                               "</iq>",
                               session.active_token, session.profile_id,
                               session.online_id, resource);
        }

        /* 2. Confirm invitation */
        send_stream_format(session.wfs,
                           "<iq to='%s' type='get'>"
                           " <query xmlns='urn:cryonline:k01'>"
                           "  <invitation_accept ticket='%s' result='0'/>"
                           " </query>"
                           "</iq>",
                           server, ticket);

        /* 3. Join the room */
        send_stream_format(session.wfs,
                           "<iq to='%s' type='get'>"
                           " <query xmlns='urn:cryonline:k01'>"
                           "  <gameroom_join room_id='%s' team_id='0' group_id='%s'"
                           "     status='1' class_id='1' join_reason='0'/>"
                           " </query>"
                           "</iq>",
                           server, room, group);

        /* 4. Change public status */
        xmpp_iq_player_status(STATUS_ONLINE | STATUS_ROOM);

        free(server);
        free(ticket);
        free(room);

        free(session.channel);
        session.channel = resource;

        free(session.group_id);
        session.group_id = group;
    }
}

void xmpp_iq_follow_send_cb(const char *msg_id, const char *msg)
{
    /* Answer:
       <iq from='xxxxx@warface/GameClient' id='uid000002c1' type='get'>
        <query xmlns='urn:cryonline:k01'>
         <follow_send nickname='xxxxxx' profile_id='xxxx'/>
        </query>
       </iq>
    */

    char *from_jid = get_info(msg, "from='", "'", NULL);
    char *nickname = get_info(msg, "nickname='", "'", NULL);

    /* Accept any follow request */
    send_stream_format(session.wfs,
                       "<iq to='masterserver@warface/%s' type='get'>"
                       " <query xmlns='urn:cryonline:k01'>"
                       "  <invitation_send nickname='%s' is_follow='1' group_id='%s'/>"
                       " </query>"
                       "</iq>",
                       session.channel, nickname, session.group_id);

    send_stream_format(session.wfs,
                       "<iq to='%s' id='%s' type='result'>"
                       " <query xmlns='urn:cryonline:k01'>"
                       "  <follow_send/>"
                       " </query>"
                       "</iq>",
                       from_jid, msg_id);

    free(nickname);
    free(from_jid);
}

/** THEADS **/

#ifdef DEBUG
void *thread_readline(void *varg)
{
    int wfs = session.wfs;

    using_history();

    do {
        char *buff_readline = readline("CMD# ");

        if (buff_readline == NULL)
            session.active = 0;
        else
        {
            int buff_size = strlen(buff_readline);

            if (buff_size <= 1)
                flush_stream(wfs);
            else
            {
                add_history(buff_readline);
                send_stream(wfs, buff_readline, buff_size);
                sleep(1);
            }
        }
    } while (session.active);

    printf("Closed readline\n");
    pthread_exit(NULL);
}
#endif

void *thread_stats(void *varg)
{
    int wfs = session.wfs;

    register_query((t_uid *) "stats", &print_number_of_players_cb, 1);

    sleep(3);

    do {
        send_stream_ascii(wfs,
                          "<iq to='conference.warface' type='get' id='stats'>"
                          " <query xmlns='http://jabber.org/protocol/disco#items'/>"
                          "</iq>");
        flush_stream(wfs);
        sleep(5);
    } while (session.active);

    printf("Closed stats\n");
    pthread_exit(NULL);
}

void *thread_dispatch(void *vargs)
{
    register_stanza("message", xmpp_message_cb);
    register_stanza("friend_list", xmpp_iq_friend_list_cb);
    register_stanza("ping", xmpp_iq_ping_cb);
    register_stanza("peer_status_update", xmpp_iq_peer_status_update_cb);
    register_stanza("invitation_request", xmpp_iq_invitation_request_cb);
    register_stanza("gameroom_sync", xmpp_iq_gameroom_sync_cb);
    register_stanza("follow_send", xmpp_iq_follow_send_cb);

    int size = 0;
    do {
        char *msg = read_stream_keep(session.wfs);

        if (msg == NULL)
            break;

        char *msg_id = get_msg_id(msg);

        /* Look if the ID is registered in the query callback handler */
        if (handle_queries(msg_id, msg))
        {
            /* Nothing to be done here */
        }
        else /* Unhandled ID */
        {
            char *stanza = get_query_tag_name(msg);

            /* Look if tagname is registered in the stanza callback handler */
            if (handle_stanza(stanza, msg_id, msg))
            {
                /* Nothing to be done here */
            }
            else /* Unhandled stanza */
            {
                /* TODO: print it for readline ? */
            }

            free(stanza);
        }

        size = strlen(msg);
        free(msg);
        free(msg_id);

    } while (size > 0 && session.active);

    session.active = 0;
    printf("Closed idle\n");
    pthread_exit(NULL);
}

void idle(void)
{
#if defined (STAT_BOT) || defined (DEBUG)

    pthread_t thread_dl;
    void * (*thread_func)(void *);

# ifdef STAT_BOT
    thread_func = &thread_stats;
# elif DEBUG
    thread_func = &thread_readline;
# endif

    if (pthread_create(&thread_dl, NULL, thread_func, NULL) == -1)
        perror("pthread_create");
    pthread_join(thread_dl, NULL);

#else /* STAT_BOT || DEBUG */

    while (session.active)
        sleep(1);

#endif /* STAT_BOT || DEBUG */
}

int main(int argc, char *argv[])
{
    if (argc <= 2)
    {
        fprintf(stderr, "USAGE: %s token online_id [eu/na/tr]\n", argv[0]);
        return 2;
    }

    char *token = argv[1];
    char *online_id = argv[2];
    char *server = "com-eu.wfw.warface.com";

    if (argc > 3)
    {
        if (strcmp(argv[3], "na") == 0)
            server = "com-us.wfw.warface.com";
        else if (strcmp(argv[3], "tr") == 0)
            server = "185.28.0.12";
    }

    session.online_id = strdup(online_id);

    int wfs = connect_wf(server, 5222);
    session.wfs = wfs;
    session.active = 1;

    /* TODO: Do handshake also in a query handler */

    /* Send Handshake */
    send_stream_ascii(wfs, "<?xml version='1.0' ?>"
                    "<stream:stream to='warface'"
                    " xmlns='jabber:client'"
                    " xmlns:stream='http://etherx.jabber.org/streams'"
                    " xml:lang='en' version='1.0'>");
    flush_stream(wfs);
    read_stream(wfs);
    read_stream(wfs);

#if 0
    /* Enable TLS connection */
    send_stream_ascii(wfs, "<starttls xmlns='urn:ietf:params:xml:ns:xmpp-tls'/>");
    flush_stream(wfs);
    read_stream(wfs);
#endif

    /* SASL Authentification */
    char *logins_b64 = combine_logins(token, session.online_id);
    send_stream_format(wfs,
                       "<auth xmlns='urn:ietf:params:xml:ns:xmpp-sasl'"
                       " mechanism='WARFACE'>"
                       "%s"
                       "</auth>",
                       logins_b64);
    read_stream(wfs);
    free(logins_b64);

    /* Open a new stream */
    send_stream_ascii(wfs,
                    "<stream:stream to='warface'"
                    " xmlns='jabber:client'"
                    " xmlns:stream='http://etherx.jabber.org/streams'"
                    " xml:lang='en' version='1.0'>");
    flush_stream(wfs);
    read_stream(wfs);
    read_stream(wfs);

    pthread_t thread_di;
    if(pthread_create(&thread_di, NULL, thread_dispatch, NULL) == -1)
        perror("pthread_create");

    xmpp_bind("GameClient");

    idle();

    if (session.active)
        pthread_join(thread_di, NULL);

    xmpp_iq_player_status(STATUS_OFFLINE);

    /* Close stream */
    send_stream_ascii(wfs, "</stream:stream>");
    flush_stream(wfs);
    close(wfs);

    free(session.jid);
    free(session.nickname);
    free(session.active_token);
    free(session.profile_id);
    free(session.online_id);
    free(session.channel);
    free(session.friend); /* TODO: List */
    free(session.group_id);

    return 0;
}

/* Old test stuff : */
void old(void){
#if 0
    /* Get master server ? */
    send_stream_ascii(wfs, "<iq from='");
    send_stream_ascii(wfs, jid);
    send_stream_ascii(wfs,
                    "' to='masterserver@warface/pvp_pro_4' type='get' id='ms_1'>"
                    "  <query xmlns='urn:cryonline:k01'>"
                    "    <get_master_server rank='45' channel='pve' is_quickplay='0'/>"
                    "  </query>"
                    "</iq>");
    flush_stream(wfs);
    read_stream(wfs);
#endif

#if 0
    /* Get clan items */
    send_stream_ascii(wfs, "<iq from='");
    send_stream_ascii(wfs, jid);
    send_stream_ascii(wfs,
                    "' to='clan.xxx@conference.warface' type='get' id='ci_1'>"
                    "  <query xmlns='http://jabber.org/protocol/disco#items'/>"
                    "</iq>");
    flush_stream(wfs);
    read_stream(wfs);
#endif

#if 0
    /* Get Mission list */
    send_stream_ascii(wfs, "<iq from='");
    send_stream_ascii(wfs, jid);
    send_stream_ascii(wfs,
                    "' to='masterserver@warface/pvp_pro_4' type='get' id='mission_list_1'>"
                    "  <query xmlns='urn:cryonline:k01'>"
                    "    <missions_get_list/>"
                    "  </query>"
                    "</iq>");
    flush_stream(wfs);
    read_stream(wfs);
#endif

#if 0
    /* Join the clan room */
    send_stream_ascii(wfs, "<presence to='clan.xxx@conference.warface/");
    send_stream_ascii(wfs, nickname);
    send_stream_ascii(wfs, "' />");
    flush_stream(wfs);
    read_stream(wfs);
#endif

#if 0
    /* Send a message to clan room */
    send_stream_ascii(wfs, "<message from='");
    send_stream_ascii(wfs, jid);
    send_stream_ascii(wfs, "' to='clan.xxx@conference.warface/");
    send_stream_ascii(wfs, nickname);
    send_stream_ascii(wfs, "' xml:lang='en' type='groupchat'>"
                    "  <body>Greatings</body>"
                    "</message>");
    flush_stream(wfs);
    read_stream(wfs);
#endif
}

/* Unresolved Queries:
-----------------------

aas_notify_playtime
autorotate
broadcast_session_result
confirm_notification
expire_profile_items profile_idle class_id time_played
external_shop_confirm_query supplierId orderId
external_shop_refund_query supplierId orderId
follow_send
invitation_accept
invitation_request
invitation_result
invitation_send
lobbychat_getchannelid channel
map_voting_finished
map_voting_started
map_voting_state
map_voting_vote mission_uid
mission_load
mission_load_failed
mission_unload
mission_update
notification_broadcast
on_voting_finished
on_voting_started
on_voting_vote
persistent_settings_get
persistent_settings_set
preinvite_invite
preinvte_response
quickplay_maplist received size
remove_friend target
resync_profile
send_invitation target type
set_banner
shop_get_offers
shop_buy_offer supplier_id offer_id
shop_buy_multiple_offer supplier_id/offer id
shop_buy_external_offer supplier_id offer_id
shop_buy_multiple_external_offer supplier_id/offer id
sponsor_info_updated
srv_player_kicked
sync_notifications
tutorial_status

validate_payer_info
voting_start target
voting_vote answer
set_reward_info session_id difficulty isPvE isClanWar mission_id incomplete_session session_time session_kill_counter winning_team_id passed_sublevels_counter passed_checkpoints_counter secondary_objectives_completed max_session_score/players_performance/stat id value
customhead_reset default_head
========= */

/* Results :
------------

friend_list
clan_info
update_cry_money
peer_clan_member_update
peer_status_update

========= */

/* Queries for ms.warface :
---------------------------

channel_logout X
items
gameroom_setgameprogress room_id game_progress
gameroom_askserver server
gameroom_join room_id team_identifier group_id status class_id
gameroom_get room_type='14' size='108' received='0' cancelled='0' token='0'
gameroom_kick target_id
gameroom_leave
gameroom_loosemaster
gameroom_offer_response
gameroom_on_expired
gameroom_on_kicked
gameroom_open [mission_data] room_name team_id status class_id room_type private mission friendly_fire enemy_outlines auto_team_balance dead_can_chat join_in_the_process max_player inventory_slot class_riffleman(enabled) [...] / session?
gameroom_promote_to_host new_host_profile_id
gameroom_quickplay room_type room_name mission_id mission_type status team_id class_id missions_hash content_hash channel_switches? timestamp? uid? group?// session
gameroom_quickplay_cancel
gameroom_quickplay_canceled
gameroom_quickplay_failed
gameroom_quickplay_started
gameroom_quickplay_succeeded
gameroom_setinfo by_mission_keywords mission_key data
gameroom_setname room_name
gameroom_setplayer team_id status class_id
gameroom_setprivatestatus private
gameroom_setteamcolor
gameroom_switchteams
gameroom_sync
gameroom_update_pvp by_mission_key mission_key [data] private
generic_telemetry X
telemetry_stream session_id packet_id finalize
get_account_profiles version='1.1.1.3522' user_id='xxxxxxxx' token='$account_token'
get_achievements/achievement profile_id='xxxx'
get_configs
get_last_seen_date profile_id='xxxxxx'
get_player_stats
ingame_event profile_id event_name [data?key/value]// error
missions_get_list
validate_player_info online_id nickname banner_badge banner_mark banner_stripe experience clan_name clan_role clan_position clan_points clan_member_since
class_presence

======== */


/* Queries for masterserver@warface/pve_12 :
--------------------------------------------

telemetry_getleaderboard limit='10' // Removed ?
admin_cmd command='' args='' // kick ban unban mute unmute
get_contracts profile_id // doesn't work with other profile_id
get_cry_money
get_profile_performance
get_storage_items
session_join
session_join_dediclient
unbind_server_info server
extend_item item_id supplier_id offer_id
get_expired_items
update_achievements[cheat_mode=1]/achievement profile_id='xxxx'/chunk achievement_id='53' progress='15813349' completion_time='0'
update_contracts
abuse_report target type comment
peer_player_info to online_id@warface/GameClient
clan_info_sync
clan_list
getprofile session_id id
setcharacter gender(female/male) height fatness current_class
setcurrentclass current_class
ui_user_choice/choice choice_id choice_result

======== */


/* Queries for k01.warface :
----------------------------

account login=userid [password=token]
create_profile version='1.1.1.3522' token='$account_token' nickname='xxxxx' profile_id='xxxx' user_id='xxxxxxxx' resource='pve_12' build_type='--release'
get_master_server rank channel resource used_resources is_quickplay
get_master_servers ?
join_channel version='1.1.1.3522' token='$account_token' profile_id='xxxx' user_id='xxxxxxxx' resource='pve_12' build_type='--release'
switch_channel version='1.1.1.3522' token='$account_token' profile_id='xxxx' user_id='xxxxxxxx' resource='pve_12' build_type='--release'
player_status prev_status='1' new_status='9' to='pve_12' // Bitfield 0:Online 1:??? 2:AFK 3:Lobby 4:Room 5:Playing 6:Shop 7:Inventory
profile_info_get_status nickname='xxxxxx'
iq/user_status[k01:presence] invalid

======== */
