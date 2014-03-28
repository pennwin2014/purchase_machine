#ifndef _P16POS_HMAC_H_
#define _P16POS_HMAC_H_

#include "sha1.h"
#include "md5.h"

enum {MD5 = 1,SHA};

#define byte unsigned char

#define MD5_DIGEST_SIZE 16
#define SHA_DIGEST_SIZE 20
#define MD5_BLOCK_SIZE 64

enum {
    IPAD    = 0x36,
    OPAD    = 0x5C,
#ifndef NO_SHA256
    INNER_HASH_SIZE = SHA256_DIGEST_SIZE,
#else
    INNER_HASH_SIZE = SHA_DIGEST_SIZE,
    SHA256          = 2,                     /* hash type unique */
#endif
    HMAC_BLOCK_SIZE = MD5_BLOCK_SIZE
};


/* hash union */
typedef union {
    MD5_CTX md5;
    SHA1Context sha;
    #ifndef NO_SHA256
        Sha256 sha256;
    #endif
} Hash;

/* Hmac digest */
typedef struct Hmac {
    Hash    hash;
    unsigned int  ipad[HMAC_BLOCK_SIZE  / sizeof(unsigned int)];  /* same block size all*/
    unsigned int  opad[HMAC_BLOCK_SIZE  / sizeof(unsigned int)];
    unsigned int  innerHash[INNER_HASH_SIZE / sizeof(unsigned int)]; /* max size */
    byte    macType;                                     /* md5 sha or sha256 */
    byte    innerHashKeyed;                              /* keyed flag */
} Hmac;


/* does init */
void HmacSetKey(Hmac*, int type, const byte* key, unsigned int keySz);
void HmacUpdate(Hmac*, const byte*, unsigned int);
void HmacFinal(Hmac*, byte*);

void SHADigest(SHA1Context* ctx,unsigned char digest[20]);

#endif // _P16POS_HMAC_H_


