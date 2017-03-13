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

#include "def.h"

#ifdef USE_TLS

# include <wb_log.h>
# include <stdlib.h>
# include <string.h>

# ifdef __MINGW32__
#  include <winsock.h>
# else
#  include <sys/socket.h>
#  include <sys/types.h>
# endif

# include <openssl/ssl.h>
# include <openssl/bio.h>
# include <openssl/err.h>

# ifdef VALGRIND_API
#  include <valgrind/memcheck.h>
# endif

static SSL_CTX* ctx = NULL;
static SSL *ssl = NULL;

typedef ssize_t (*f_recv)(int fd, void *buf, size_t count);
typedef ssize_t (*f_send)(int fd, const void *buf, size_t count);
typedef void (*f_perror)(const char *s, int ret);

static ssize_t _default_recv(int fd, void *buf, size_t count)
{
    return recv(fd, buf, count, 0);
}

static ssize_t _tls_recv(int fs, void *buf, size_t count)
{
    ssize_t status = SSL_read(ssl, buf, count);

    /*
    ** Valgrind thinks buf still contains uninitialized
    ** bits after a call to SSL_read, whereas it's an
    ** expected behavior from OpenSSL. Tell Valgrind to
    ** forget about it.
    */
# ifdef VALGRIND_MAKE_MEM_DEFINED
    if (status > 0)
        VALGRIND_MAKE_MEM_DEFINED(buf, status);
# endif

    return status;
}

static ssize_t _default_send(int fd, const void *buf, size_t count)
{
    return send(fd, buf, count, MSG_MORE);
}

static ssize_t _tls_send(int fd, const void *buf, size_t count)
{
    return SSL_write(ssl, buf, count);
}

static void _default_perror(const char *s, int ret)
{
    perror(s);
}

static void _tls_perror(const char *s, int ret)
{
    const char *cause = NULL;
    int err = SSL_get_error(ssl, ret);

    switch(err)
    {
        case SSL_ERROR_NONE:
            cause = "Success";
            break;
        case SSL_ERROR_SYSCALL:
            if (ret == 0)
                cause = "End of stream";
            else
                cause = "I/O error";
            break;
        case SSL_ERROR_SSL:
            cause = "Protocol error";
            break;
        case SSL_ERROR_WANT_READ:
            cause = "End of stream";
            break;
        default:
            cause = "Unknown error";
    }

    eprintf("%s: %s (%i)", s, cause, err);
}

static f_recv _recv_proc = _default_recv;
static f_send _send_proc = _default_send;
static f_perror _perror_proc = _default_perror;

static int init_error(void)
{
    char *buf = NULL;
    BIO * bio = BIO_new(BIO_s_mem());

    ERR_print_errors(bio);

    size_t len = BIO_get_mem_data(bio, &buf);
    char *ret = calloc(1, 1 + len);

    if (ret != NULL)
        memcpy(ret, buf, len);

    BIO_free(bio);

    eprintf("error!\n%s\n", ret);

    free(ret);

    return 1;
}

int tls_init(int fd)
{
    const SSL_METHOD* method;

    xprintf("Starting TLS connection...");

    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();

    if ((method = TLSv1_method()) == NULL)
        return init_error();

    if ((ctx = SSL_CTX_new(method)) == NULL)
        return init_error();

    SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);

    if ((ssl = SSL_new(ctx)) == NULL)
        return init_error();

    if (SSL_set_fd(ssl, fd) != 1)
        return init_error();

    if (SSL_connect(ssl) != 1)
        return init_error();

    if (SSL_do_handshake(ssl) != 1)
        return init_error();

    _recv_proc = _tls_recv;
    _send_proc = _tls_send;
    _perror_proc = _tls_perror;

    xprintf("done.");

    return 0;
}

ssize_t tls_recv(int fd, void *buf, size_t count)
{
    return _recv_proc(fd, buf, count);
}

ssize_t tls_send(int fd, const void *buf, size_t count)
{
    return _send_proc(fd, buf, count);
}

void tls_perror(const char *s, int ret)
{
    return _perror_proc(s, ret);
}

void tls_close(void)
{
    if (ssl != NULL)
        SSL_shutdown(ssl);
}

void tls_free(void)
{
    if (ssl != NULL)
        SSL_free(ssl);

    if (ctx != NULL)
        SSL_CTX_free(ctx);
}
#endif /* USE_TLS */
