/**
 * WarfaceBot, a blind XMPP client for Warface (FPS)
 * Copyright (C) 2015, 2016 Levak Borok <levak92@gmail.com>
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

#include <wake.h>
#include <wb_stream.h>

static uint32_t crypt_key[] = {
    0x65, 0x18, 0x8E, 0x00, 0x64, 0x19, 0x8F, 0x01,
    0x67, 0x1A, 0x8C, 0x02, 0x66, 0x1B, 0x8D, 0x03,
    0x69, 0x14, 0x82, 0x0C, 0x68, 0x15, 0x83, 0x0D,
    0x6B, 0x16, 0x80, 0x0E, 0x6A, 0x17, 0x81, 0x0F,
};

static uint32_t crypt_iv[] = {
    0x1f, 0x78, 0xd4, 0xf4, 0x22, 0x56, 0xf9, 0x19,
};

static const int crypt_key_len = sizeof (crypt_key) / sizeof (crypt_key[0]);
static const int crypt_iv_len = sizeof (crypt_iv) / sizeof (crypt_iv[0]);

static int crypt_ready = 0;

int crypt_is_ready(void)
{
    return crypt_ready;
}

void crypt_init(int key)
{
    int i = 0;

    for (; i < crypt_iv_len; ++i)
        crypt_iv[i] ^= key;

    crypt_ready = 1;
}

void crypt_decrypt(uint8_t *buff, int len)
{
    if (crypt_ready)
    {
        WAKE_KEY m;

        mcrypt_set_key(&m, crypt_key, crypt_key_len, crypt_iv, crypt_iv_len);
        mcrypt_decrypt(&m, buff, len);
    }
    else
    {
        /* Nothing to do */
        return;
    }
}

void crypt_encrypt(uint8_t *buff, int len)
{
    if (crypt_ready)
    {
        WAKE_KEY m;

        mcrypt_set_key(&m, crypt_key, crypt_key_len, crypt_iv, crypt_iv_len);
        mcrypt_encrypt(&m, buff, len);
    }
    else
    {
        /* Nothing to do */
        return;
    }
}
