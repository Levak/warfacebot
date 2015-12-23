#ifndef WAKE_H
# define WAKE_H

# include <stdint.h>
# include <stdlib.h>
# include <stdio.h>
# include <string.h>

# define word32 uint32_t
# define byte uint8_t

# define WIN32DLL_DEFINE
# define inline inline

# define mcrypt_set_key _mcrypt_set_key
# define mcrypt_encrypt _mcrypt_encrypt
# define mcrypt_decrypt _mcrypt_decrypt

typedef struct wake_key {
	word32 t[257];
	word32 r[4];
	int counter;
	word32 tmp; /* used as r1 or r2 */
	int started;
	word32 iv[8];
	int ivsize;
} WAKE_KEY;

int _mcrypt_set_key(WAKE_KEY * wake_key,
                    word32 * key, int len,
                    word32 * IV, int ivlen);

void _mcrypt_encrypt(WAKE_KEY * wake_key, byte * input, int len);
void _mcrypt_decrypt(WAKE_KEY * wake_key, byte * input, int len);

#endif /* !WAKE_H */
