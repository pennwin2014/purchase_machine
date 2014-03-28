#ifndef _P16POS_CARDOP_H_
#define _P16POS_CARDOP_H_

// m1 ¿¨
#define CT_M1 0x01
// CPU ¿¨
#define CT_CPU 0x02

#include "config.h"

int8 request_card(uint8 cardphyid[],uint8 *cardtype);
int8 request_card_and_poweron(uint8 cardphyid[],uint8 *cardtype);
int8 read_user_card(p16_card_context* cardctx,char emsg[256]);
int8 read_user_purchase_card(p16_card_context* cardctx,char emsg[256]);

int8 init_4_load(p16_card_context* cardctx);
int8 debit_4_load(p16_card_context* cardctx);

int8 init_4_purchase(p16_card_context* cardctx);
int8 debit_4_purchase(p16_card_context* cardctx);
int8 psam_purchase_mac(p16_card_context* cardctx);

int8 get_card_prove(p16_card_context* cardctx);
int8 sam_read_termno(uint8* termno);


#endif // _P16POS_CARDOP_H_
