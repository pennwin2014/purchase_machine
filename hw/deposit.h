#ifndef _P16_DEPOSIT_H_
#define _P16_DEPOSIT_H_

#include "config.h"
#include "syspara.h"
void deposit_main();
void purchase_main();

int check_oper_login();
int pos_printer(p16_transdtl_t* transdtl, int page, int pagecnt);

int check_blackcard(int cardno);

#endif // _P16_DEPOSIT_H_

