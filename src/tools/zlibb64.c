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

#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/comp.h>

#include <string.h>

char *zlibb64encode(const void *input, size_t inlength)
{ /* Untested */
#ifdef ZLIB
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
#else /* ZLIB */
    return NULL;
#endif /* ZLIB */
}

char *zlibb64decode(const void *input, size_t inlength, size_t outlength)
{
#ifdef ZLIB
    BIO *bmem;
    BIO *bz;
    BIO *b64;
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
#else /* ZLIB */
    return NULL;
#endif /* ZLIB */
}
