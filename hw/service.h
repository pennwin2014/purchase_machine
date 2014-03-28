#ifndef _P16POS_SERVICE_H_
#define _P16POS_SERVICE_H_

#include "config.h"
#include "syspara.h"

#define SVC_OK 0
#define SVC_FAILED -1
#define SVC_TIMEOUT -2

#define FLAG_TRANSACTION 0
#define FLAG_QUERY 1

void setup_svc_url();
int svc_app_login();
int svc_oper_login(const char* cardphyid, const char* operpwd, int* cfgverno);
int svc_deposit(p16_transdtl_t* transdtl);
int svc_deposit_query(p16_transdtl_t* transdtl);
int svc_deposit_confirm(p16_transdtl_t* transdtl , int* nexttermseqno);
int svc_get_download_dps_para(const char* devphyid);

int svc_allowance_machine_login();
int svc_query_allowance(allowance_info_t* allowance);
int svc_get_allowance(p16_transdtl_t* transdtl, allowance_info_t* allowance, int flag);
#endif // _P16POS_SERVICE_H_

