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

#include <wb_stream.h>
#include <wb_session.h>
#include <wb_cvar.h>

#include <wake.h>
#include <stdlib.h>

static uint32_t crypt_key[32];
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

void crypt_init(int salt)
{
    int i = 0;

    for (; i < crypt_iv_len; ++i)
        crypt_iv[i] ^= salt;

    if (cvar.game_version != NULL)
    {
        const char *p = cvar.game_version;

        int ver[4];

        for (int i = 0; i < 4; ++i)
        {
            char *end;

            ver[i] = strtol(p, &end, 10);

            if (end == NULL)
                break;

            p = end + 1;
        }

        crypt_key[0] = (ver[0] ^ ver[3]) & 0xFF;
        crypt_key[1] = (ver[1] ^ ver[3]) & 0xFF;
        crypt_key[2] = (ver[2] ^ ver[3]) & 0xFF;
        crypt_key[3] = (ver[3] ^ ver[3]) & 0xFF;
    }

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
