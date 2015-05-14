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

#include <string.h>

#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/comp.h>

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
