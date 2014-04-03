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
/*
*socket部分
*/
int tcp_demo_send();//测试的demo
int tcp_send_data(char* send_buf, uint32 send_len, char* recv_buf, uint32* recv_len);//发送函数
void pack_tcp_rt_transdtl_data(p16_tcp_rt_package* package, char* send_buf, uint32* send_len);
void pack_tcp_batch_data(p16_tcp_batch_package* batch_package, char* send_buf, uint32* send_len);
void pack_tcp_heart_data(p16_tcp_heart_package* heart_package, char* send_buf, uint32* send_len);
void pack_tcp_blacklist_data(p16_tcp_blacklist_package* package, char* send_buf, uint32* send_len);

void pb_protocol_crc(const uint8* buffer, uint16 buffer_len, uint8 crc[2]);




#endif // _P16POS_SERVICE_H_

