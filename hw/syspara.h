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
*socket���͵Ľ�����ˮ�ṹ
*/
#pragma pack(1) 
/////////////////////////////////////////�������/////////////////////////////////

//////////////////���յ�unit/////////////////////////////
typedef struct
{
	uint8 refno[4];//��ˮ��
	uint8 flag;//��־λ
}p16_recv_blacklist_unit;
//////////////////���յ�Data/////////////////////////////

typedef struct
{
	uint8 ret_code;//������
	uint8 current_blacklist_version[6];//��ǰϵͳ�������汾��
	uint8 server_datetime[6];//������ʱ��
	uint8 cfgverno[4];//ϵͳ�����汾
}p16_recv_heart_data;


typedef struct
{
	uint8 ret_code;//������
}p16_recv_rt_transdtl_data;

typedef struct
{
	uint8 ret_code;//������
}p16_recv_batch_transdtl_data;

typedef struct
{
	uint8 list_cnt;//������
	p16_recv_blacklist_unit blacklist_list[MAX_RECV_BLACKLIST_CNT];
	uint8 sys_list_version[6];//ϵͳ�����汾��
}p16_recv_blacklist_data;


///////////////////���յ�package/////////////////////////
typedef struct
{
	char guide_code[3];//������
	uint8 cmd_code;//������
	uint16 package_len;//���ݰ�����
	uint8 machine_addr[2];//������ַ
	p16_recv_heart_data data;//����
	uint8 check;//У��
}p16_recv_heart_package;

typedef struct
{
	char guide_code[3];//������
	uint8 cmd_code;//������
	uint16 package_len;//���ݰ�����
	uint8 machine_addr[2];//������ַ
	p16_recv_rt_transdtl_data data;//����
	uint8 check;//У��
}p16_recv_rt_transdtl_package;


typedef struct
{
	char guide_code[3];//������
	uint8 cmd_code;//������
	uint16 package_len;//���ݰ�����
	uint8 machine_addr[2];//������ַ
	p16_recv_batch_transdtl_data data;//����
	uint8 check;//У��
}p16_recv_batch_transdtl_package;


typedef struct
{
	char guide_code[3];//������
	uint8 cmd_code;//������
	uint16 package_len;//���ݰ�����
	uint8 machine_addr[2];//������ַ
	p16_recv_blacklist_data data;//����
	uint8 check;//У��

}p16_recv_blacklist_package;


////////////////////////////////////////�������//////////////////////////////

////////////////data����////////////////
typedef struct
{
	uint32 devseqno;//pos���ļ�¼��ˮ��devseqno
	uint8 cardno[4];//�û�����ˮ��(�����׿���)
	uint8 cardbefbal[4];//����ǰ���
	uint8 paycnt[2]; //����ǰ����
	uint8 transamt[4];//����ʵ�ʽ��(�������Ѻ��ۿ�)
	uint8 additional[3];//���ѻ����ۿ۽��
	uint8 transdatetime[6];//��������ʱ��(YYMMDDHHMMSS)
	uint8 termno[6];//psam�����࣬���ڱ��ص�termno
	uint8 tac[4];//tac��
	uint8 machine_no[4];//���ţ�00
	uint8 transflag;//���ױ�־
	uint8 reserve[4];//����
	uint8 crc[2];//CRCУ��
}p16_tcp_transdtl;

typedef struct
{
	uint8 tcp_transdtl_cnt;
	p16_tcp_transdtl tcp_transdtl_list[MAX_TCP_TRANSDTL_CNT];
}p16_tcp_batch_data;

typedef struct
{
	uint8 identify_code;//0x01���� 0x02����
	uint8 machine_no[4];//����4�ֽ�
	uint8 cfgverno[4];//�豸�����汾��
}p16_tcp_heart_data;


typedef struct
{
	uint8 pos_blacklist_version[6];//pos�������汾
	uint8 machine_no[4];//����
}p16_tcp_blacklist_data;

////////////////////�����İ�////////////////////////////
typedef struct
{
	char guide_code[3];//������
	uint8 cmd_code;//������
	uint16 package_len;//���ݰ�����
	uint8 machine_addr[2];//������ַ
	p16_tcp_batch_data data;//����
	uint8 check;//У��
}p16_tcp_batch_package;

typedef struct
{
	char guide_code[3];//������
	uint8 cmd_code;//������
	uint16 package_len;//���ݰ�����
	uint8 machine_addr[2];//������ַ
	p16_tcp_transdtl data;//����
	uint8 check;//У��
}p16_tcp_rt_package;

typedef struct
{
	char guide_code[3];//������
	uint8 cmd_code;//������
	uint16 package_len;//���ݰ�����
	uint8 machine_addr[2];//������ַ
	p16_tcp_heart_data data;//����
	uint8 check;//У��
}p16_tcp_heart_package;

typedef struct
{
	char guide_code[3];//������
	uint8 cmd_code;//������
	uint16 package_len;//���ݰ�����
	uint8 machine_addr[2];//������ַ
	p16_tcp_blacklist_data data;//����
	uint8 check;//У��
}p16_tcp_blacklist_package;

#pragma pack()

/**
 * @brief �ڿ���
 */
typedef struct
{
    int cardno;
    uint8 cardflag;
    char remark[256];
}p16_blackcard_t;


int8 blackcard_add_record(p16_blackcard_t* record);
int8 blackcard_update_record(p16_blackcard_t* record);
int8 blackcard_get_record_by_cardno(p16_blackcard_t* record, int cardno);


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

