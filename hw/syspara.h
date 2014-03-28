#ifndef _P16POS_SYSPARA_H_
#define _P16POS_SYSPARA_H_

#include "config.h"

#define DB_OK 0
#define DB_ERROR -1
#define DB_NOTFOUND 2

int8 check_config_db();
void close_config_db();
int8 update_config(const char* paraname, char* paraval);
int8 update_config_int(const char* paraname, int paraval);
int8 get_config(const char* paraname, char* paraval, int maxlen);
int8 get_config_int(const char* paraname, int* paraval);
int8 syspara_reset();


#define TRANS_INIT 0 // ��ʼ
#define TRANS_WRITE_CARD_OK 1 // д���ɹ�
#define TRANS_WRITE_CARD_FAILED 2 // д��ʧ��
#define TRANS_WRITE_CARD_UNCONFIRM 3 // ��;�ο�
#define TRANS_SYS_OK 4 // ϵͳ�ɹ�
#define TRANS_SYS_FAILED 5 // ϵͳʧ��

/**
 * @brief ����Ա��ֵ��ˮ
 */
typedef struct
{
    int operid;
    int cardno;
    char cardphyid[17];
    char termno[13];
    int termid;
    char devphyid[17];
    int devseqno;
    int cardbefbal;
    int amount;
    int dpsamt;
    int paycnt;
    int dpscnt;
    char transdate[9];
    char transtime[9];
    char refno[21];
    int transflag;
    int confirm;
    char tac[9];
    int authcode;
    int batchno;
    // �����ֶβ���¼����ˮ����
    char random[9];
    char mac1[9];
    char hostdate[9];
    char hosttime[9];
    char mac2[9];
    char errmsg[256];
    int errcode;
} p16_transdtl_t;
typedef struct
{
    int operid;
    int cardno;
    char cardphyid[17];
    char termno[13];
    int termid;
    char devphyid[17];
    int devseqno;
    int cardbefbal;
    int amount;
    int dpsamt;
	int payamt;//==20140327�����
    int paycnt;
    int dpscnt;
    char transdate[9];
    char transtime[9];
    char refno[21];
    int transflag;
    int confirm;
    char tac[9];
    int authcode;
    int batchno;
    // �����ֶβ���¼����ˮ����
    char random[9];
    char mac1[9];
    char hostdate[9];
    char hosttime[9];
    char mac2[9];
    char errmsg[256];
    int errcode;
	//
	
} p16_purchase_transdtl_t;


int8 trans_add_record(p16_transdtl_t* record);
int8 trans_get_last_record(p16_transdtl_t* record);
int8 trans_update_record(p16_transdtl_t* record);
int8 trans_clear_expire_dtl(const char* daybefore);
int8 trans_clear_dtl();
int8 trans_get_oper_devseqno(p16_transdtl_t* record, int operid, int devseqno);
int8 trans_get_last_succ(p16_transdtl_t* record, int operid);

typedef struct
{
    char transdate[9];
    int total_cnt;
    int total_amt;
    int success_cnt;
    int success_amt;
} oper_trans_t;
int8 oper_get_trans(const char* transdate, int operid, oper_trans_t* opertrans);

typedef struct
{
    int feetype;
    int feerate;
    int feeflag;
} syspara_feerate_t;
int8 syspara_get_feerate(int feetype, syspara_feerate_t* feerate);
int8 syspara_clear_feerate();
int8 syspara_add_feerate(const syspara_feerate_t* feerate);


/**
 * @brief ������ϸ����
 */
typedef struct
{
    int subsidyno;
    int amount;
} allowance_detail_t;

/**
 * @brief ������Ϣ
 */
typedef struct
{
    int total_amount;
    int total_get_amt;
    int dps_amt;
    int allowance_count;
    allowance_detail_t* detail;
} allowance_info_t;

int syspara_add_allowance_detail(p16_transdtl_t* dtl, allowance_info_t* info);
int syspara_get_allowance_detail(p16_transdtl_t* dtl, allowance_info_t* info);

#define SAFE_GET_CONFIG(n,v) get_config(n,v,sizeof(v)-1)

#endif // _P16POS_SYSPARA_H_

