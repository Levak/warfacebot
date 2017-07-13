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
        int ver_len = sizeof (ver) / sizeof (ver[0]);

        for (int i = 0; i < ver_len; ++i)
        {
            char *end;

            ver[i] = strtol(p, &end, 10);

            if (end == NULL)
                break;

            p = end + 1;
        }

        for (int j = 0; j < 8; ++j)
            for (int i = 0; i < ver_len; ++i)
                crypt_key[i + (j * 4)] =
                    (ver[i] ^ (ver[ver_len - 1] + j)) & 0xFF;
    }

    if (cvar.game_crypt_key != NULL)
    {
        const char *p = cvar.game_crypt_key;

        for (int i = 0; i < crypt_key_len; ++i)
        {
            char *end;

            crypt_key[i] = strtol(p, &end, 10);

            if (end == NULL)
                break;

            p = end + 1;
        }
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
