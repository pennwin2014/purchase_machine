/******************************************************************************
* Copyright SupWisdom 2012 all right reserved
* Module: SHA algorithem
* File: hmac.c
* Auth: Cheng Tang<cheng.tang@supwisdom.com> 
******************************************************************************/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "hmac.h"

void SHADigest(SHA1Context* ctx, unsigned char digest[20])
{
  /*
  int i,j;
  char temp[9] = {0};
  char ultemp[3] = {0};
  for(i = 0;i < 5;++i)
  {
  	sprintf(temp,"%08X",ctx->Message_Digest[i]);
    for(j = 0;j < sizeof(unsigned int); ++j)
    {
      memcpy(ultemp,temp+j*2,2);
      digest[i*4+j] = (unsigned char)strtoul(ultemp,NULL,16);
    }
  }
  */
  int i;
  const unsigned char* md = (const unsigned char*) & (ctx->Message_Digest);
  for(i = 0; i < 5; ++i)
  {
    digest[(i << 2)] = md[(i << 2) + 3];
    digest[(i << 2) + 1] = md[(i << 2) + 2];
    digest[(i << 2) + 2] = md[(i << 2) + 1];
    digest[(i << 2) + 3] = md[(i << 2)];
  }

}

static int InitHmac(Hmac* hmac, int type)
{
  hmac->innerHashKeyed = 0;
  hmac->macType = (byte)type;

  if(!(type == MD5 || type == SHA || type == SHA256))
    return -1;

  if(type == MD5)
    MD5Init(&(hmac->hash.md5));
  else if(type == SHA)
    SHA1Reset(&(hmac->hash.sha));
#ifndef NO_SHA256
  else if(type == SHA256)
    InitSha256(&hmac->hash.sha256);
#endif

  return 0;
}


void HmacSetKey(Hmac* hmac, int type, const byte* key, unsigned int length)
{
  byte*  ip = (byte*) hmac->ipad;
  byte*  op = (byte*) hmac->opad;
  unsigned int i;

  InitHmac(hmac, type);

  if(length <= HMAC_BLOCK_SIZE)
    memcpy(ip, key, length);
  else
  {
    if(hmac->macType == MD5)
    {
      MD5Update(&hmac->hash.md5, (byte*)key, length);
      MD5Final(ip, &hmac->hash.md5);
      MD5Init(&hmac->hash.md5);
      length = MD5_DIGEST_SIZE;
    }
    else if(hmac->macType == SHA)
    {
      SHA1Input(&hmac->hash.sha, key, length);
      SHADigest(&hmac->hash.sha, ip);
      SHA1Reset(&hmac->hash.sha);
      length = SHA_DIGEST_SIZE;
    }
#ifndef NO_SHA256
    else if(hmac->macType == SHA256)
    {
      Sha256Update(&hmac->hash.sha256, key, length);
      Sha256Final(&hmac->hash.sha256, ip);
      length = SHA256_DIGEST_SIZE;
    }
#endif
  }
  memset(ip + length, 0, HMAC_BLOCK_SIZE - length);

  for(i = 0; i < HMAC_BLOCK_SIZE; i++)
  {
    op[i] = ip[i] ^ OPAD;
    ip[i] ^= IPAD;
  }
}


static void HmacKeyInnerHash(Hmac* hmac)
{
  if(hmac->macType == MD5)
    MD5Update(&hmac->hash.md5, (byte*) hmac->ipad, HMAC_BLOCK_SIZE);
  else if(hmac->macType == SHA)
    SHA1Input(&(hmac->hash.sha), (byte*) hmac->ipad, HMAC_BLOCK_SIZE);
#ifndef NO_SHA256
  else if(hmac->macType == SHA256)
    Sha256Update(&hmac->hash.sha256, (byte*) hmac->ipad, HMAC_BLOCK_SIZE);
#endif

  hmac->innerHashKeyed = 1;
}


void HmacUpdate(Hmac* hmac, const byte* msg, unsigned int length)
{
  if(!hmac->innerHashKeyed)
    HmacKeyInnerHash(hmac);

  if(hmac->macType == MD5)
    MD5Update(&hmac->hash.md5, (byte*)msg, length);
  else if(hmac->macType == SHA)
    SHA1Input(&hmac->hash.sha, msg, length);
#ifndef NO_SHA256
  else if(hmac->macType == SHA256)
    Sha256Update(&hmac->hash.sha256, msg, length);
#endif

}


void HmacFinal(Hmac* hmac, byte* hash)
{
  if(!hmac->innerHashKeyed)
    HmacKeyInnerHash(hmac);

  if(hmac->macType == MD5)
  {
    MD5Final((byte*) hmac->innerHash, &hmac->hash.md5);
    MD5Init(&hmac->hash.md5);
    MD5Update(&hmac->hash.md5, (byte*) hmac->opad, HMAC_BLOCK_SIZE);
    MD5Update(&hmac->hash.md5, (byte*) hmac->innerHash, MD5_DIGEST_SIZE);

    MD5Final(hash, &hmac->hash.md5);
  }
  else if(hmac->macType == SHA)
  {
    SHA1Result(&hmac->hash.sha);
    SHADigest(&hmac->hash.sha, (byte*) hmac->innerHash);
    SHA1Reset(&hmac->hash.sha);
    SHA1Input(&hmac->hash.sha, (byte*) hmac->opad, HMAC_BLOCK_SIZE);
    SHA1Input(&hmac->hash.sha, (byte*) hmac->innerHash, SHA_DIGEST_SIZE);
    SHA1Result(&hmac->hash.sha);
    SHADigest(&hmac->hash.sha, hash);
  }
#ifndef NO_SHA256
  else if(hmac->macType == SHA256)
  {
    Sha256Final(&hmac->hash.sha256, (byte*) hmac->innerHash);

    Sha256Update(&hmac->hash.sha256, (byte*) hmac->opad, HMAC_BLOCK_SIZE);
    Sha256Update(&hmac->hash.sha256, (byte*) hmac->innerHash,
                 SHA256_DIGEST_SIZE);

    Sha256Final(&hmac->hash.sha256, hash);
  }
#endif

  hmac->innerHashKeyed = 0;
}
