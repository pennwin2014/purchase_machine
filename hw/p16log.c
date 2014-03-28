/******************************************************************************
* Copyright SupWisdom 2012 all right reserved
* Module: Debug Log
* File: p16log
* Auth: Cheng Tang<cheng.tang@supwisdom.com>
******************************************************************************/
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "p16log.h"
#include "utils.h"

void error_exit(int errcode, const char* msg)
{
    printf("Error :%s\n", msg);
    exit(errcode);
}

void p16log(int level, const char* fmt, ...)
{
    va_list arg;
    va_start(arg, fmt);
    char msg[1024 + 1];
	memset(msg, 0, sizeof msg);
    vsnprintf( msg, sizeof( msg ) - 1, fmt, arg );
	printf(msg);
    //fprintf(stdout, fmt, arg);
    va_end(arg);
    printf("\n");
}

void p16dump_hex(int level, unsigned char* data, unsigned int len)
{
    char temp[33];
    int i, j;
    printf("DUMP HEX Length %d \n", len);
    printf("**** 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F ****\n");
    for(i = 0; i < len; i += 16)
    {
        uint8 padlen = len - i;
        if(padlen > 16)
            padlen = 16;

        memset(temp, 0, sizeof temp);
        encode_hex(data + i, padlen, temp);
        printf("     ");
        char t[4];
        memset(t, 0, sizeof t);
        t[2] = ' ';
        for(j = 0; j < padlen; ++j)
        {
            memcpy(t, temp + (j << 1), 2);
            printf(t);
        }
        printf("\n");
    }
    printf("\n");
}

