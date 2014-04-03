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


#define TRANS_INIT 0 // 初始
#define TRANS_WRITE_CARD_OK 1 // 写卡成功
#define TRANS_WRITE_CARD_FAILED 2 // 写卡失败
#define TRANS_WRITE_CARD_UNCONFIRM 3 // 中途拔卡
#define TRANS_SYS_OK 4 // 系统成功
#define TRANS_SYS_FAILED 5 // 系统失败
/**
*socket发送的交易流水结构
*/
#pragma pack(1) 
/////////////////////////////////////////接收相关/////////////////////////////////

//////////////////接收的unit/////////////////////////////
typedef struct
{
	uint8 refno[4];//流水号
	uint8 flag;//标志位
}p16_recv_blacklist_unit;
//////////////////接收的Data/////////////////////////////

typedef struct
{
	uint8 ret_code;//返回码
	uint8 current_blacklist_version[6];//当前系统黑名单版本号
	uint8 server_datetime[6];//服务器时间
	uint8 cfgverno[4];//系统参数版本
}p16_recv_heart_data;


typedef struct
{
	uint8 ret_code;//返回码
}p16_recv_rt_transdtl_data;

typedef struct
{
	uint8 ret_code;//返回码
}p16_recv_batch_transdtl_data;

typedef struct
{
	uint8 list_cnt;//名单数
	p16_recv_blacklist_unit blacklist_list[MAX_RECV_BLACKLIST_CNT];
	uint8 sys_list_version[6];//系统名单版本号
}p16_recv_blacklist_data;


///////////////////接收的package/////////////////////////
typedef struct
{
	char guide_code[3];//引导码
	uint8 cmd_code;//命令码
	uint16 package_len;//数据包长度
	uint8 machine_addr[2];//机器地址
	p16_recv_heart_data data;//数据
	uint8 check;//校验
}p16_recv_heart_package;

typedef struct
{
	char guide_code[3];//引导码
	uint8 cmd_code;//命令码
	uint16 package_len;//数据包长度
	uint8 machine_addr[2];//机器地址
	p16_recv_rt_transdtl_data data;//数据
	uint8 check;//校验
}p16_recv_rt_transdtl_package;


typedef struct
{
	char guide_code[3];//引导码
	uint8 cmd_code;//命令码
	uint16 package_len;//数据包长度
	uint8 machine_addr[2];//机器地址
	p16_recv_batch_transdtl_data data;//数据
	uint8 check;//校验
}p16_recv_batch_transdtl_package;


typedef struct
{
	char guide_code[3];//引导码
	uint8 cmd_code;//命令码
	uint16 package_len;//数据包长度
	uint8 machine_addr[2];//机器地址
	p16_recv_blacklist_data data;//数据
	uint8 check;//校验

}p16_recv_blacklist_package;


////////////////////////////////////////发送相关//////////////////////////////

////////////////data部分////////////////
typedef struct
{
	uint32 devseqno;//pos机的记录流水号devseqno
	uint8 cardno[4];//用户卡流水号(即交易卡号)
	uint8 cardbefbal[4];//交易前余额
	uint8 paycnt[2]; //交易前次数
	uint8 transamt[4];//交易实际金额(包括搭伙费和折扣)
	uint8 additional[3];//搭伙费或者折扣金额
	uint8 transdatetime[6];//交易日期时间(YYMMDDHHMMSS)
	uint8 termno[6];//psam卡号相，当于本地的termno
	uint8 tac[4];//tac码
	uint8 machine_no[4];//机号，00
	uint8 transflag;//交易标志
	uint8 reserve[4];//保留
	uint8 crc[2];//CRC校验
}p16_tcp_transdtl;

typedef struct
{
	uint8 tcp_transdtl_cnt;
	p16_tcp_transdtl tcp_transdtl_list[MAX_TCP_TRANSDTL_CNT];
}p16_tcp_batch_data;

typedef struct
{
	uint8 identify_code;//0x01消费 0x02考勤
	uint8 machine_no[4];//机号4字节
	uint8 cfgverno[4];//设备参数版本号
}p16_tcp_heart_data;


typedef struct
{
	uint8 pos_blacklist_version[6];//pos黑名单版本
	uint8 machine_no[4];//机号
}p16_tcp_blacklist_data;

////////////////////最外层的包////////////////////////////
typedef struct
{
	char guide_code[3];//引导码
	uint8 cmd_code;//命令码
	uint16 package_len;//数据包长度
	uint8 machine_addr[2];//机器地址
	p16_tcp_batch_data data;//数据
	uint8 check;//校验
}p16_tcp_batch_package;

typedef struct
{
	char guide_code[3];//引导码
	uint8 cmd_code;//命令码
	uint16 package_len;//数据包长度
	uint8 machine_addr[2];//机器地址
	p16_tcp_transdtl data;//数据
	uint8 check;//校验
}p16_tcp_rt_package;

typedef struct
{
	char guide_code[3];//引导码
	uint8 cmd_code;//命令码
	uint16 package_len;//数据包长度
	uint8 machine_addr[2];//机器地址
	p16_tcp_heart_data data;//数据
	uint8 check;//校验
}p16_tcp_heart_package;

typedef struct
{
	char guide_code[3];//引导码
	uint8 cmd_code;//命令码
	uint16 package_len;//数据包长度
	uint8 machine_addr[2];//机器地址
	p16_tcp_blacklist_data data;//数据
	uint8 check;//校验
}p16_tcp_blacklist_package;

#pragma pack()

/**
 * @brief 黑卡表
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
 * @brief 操作员充值流水
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
    // 以下字段不记录在流水表中
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
	int payamt;//==20140327新添加
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
    // 以下字段不记录在流水表中
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
 * @brief 补助明细数据
 */
typedef struct
{
    int subsidyno;
    int amount;
} allowance_detail_t;

/**
 * @brief 补助信息
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

