#ifndef _SW_P16_CONFIG_H_
#define _SW_P16_CONFIG_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "decard/port_cfg.h"
#include "decard/tft.h"
#include "decard/ds_mem.h"
#include "decard/dc_graph.h"
#include "decard/lcd160.h"
#include "decard/label.h"
#include "decard/pic_list.h"
#include "decard/input.h"
#include "decard/list.h"
#include "decard/choise.h"
#include "decard/ds_mem.h"
#include "decard/key_led.h"
#include "decard/fs_print.h"
#include "decard/rf_card.h"
#include "decard/other.h"


#define DESKTOP_JPG "../res/jpg/p16deposit_desktop.jpg"

// 是否支持 SAM 卡
#define ENABLE_SAM 1
// 是否支持打印机
#define ENABLE_PRINTER 1
// 是否支持GPRS
#define ENABLE_GPRS 1
// 是否支持CDMA
#define ENABLE_CDMA 1
// 是否支持MODEM
#define ENABLE_MODEM 0

// 是否使用 M1 卡
#define CARDTYPE_M1
// 是否使用 CPU 卡
#define CRADTYPE_CPU

// 定义 CPU 默认 PIN
#define CPU_DEFAULT_PIN "000000"

#define DEBUG_LOG 1

#define DEFAULT_DEVPHYID_LEN 8

#define DEFAULT_HZ_FONT ST_32DZ
#define DEFAULT_ASC_FONT ASC_32DZ

#define INPUT_HZ_FONT ST_24DZ
#define INPUT_ASC_FONT ASC_24DZ

#define REMOTE_WIRE 0 // 有线网络
#define REMOTE_GPRS 1 // GPRS 网络
#define REMOTE_CDMA 2 // CDMA 网络
#define REMOTE_MODEM 3 // MODEM 网络
#define REMOTE_COM  4 // COM 口
#define REMOTE_WIFI 5 // 无线网络
#define REMOTE_END 6

// 补助机模式下固定操作员ID
#define ALLOWANCE_OPER_ID 0

enum {RT_IP = 0, RT_COM, RT_HOST};

typedef struct
{
    uint8 remote_type;
    union
    {
        struct ip_addr
        {
            char remote_addr[21];
            uint32 remote_port;
        } ip;
        struct com_addr
        {
            uint8 com_num;
            uint16 com_band;
        } com;
        struct host_addr
        {
            char host_name[1024];
            uint32 host_port;
        } host;
    } u;
} p16_remote_conf;

typedef struct
{
    // appid
    char appid[17];
    // appsecret
    char appsecret[49];
    // 签到后的session key
    char session_key[65];
    uint8 device_key[16];
    // 启动路径
    char base_dir[1024];
    // 设备物理ID
    char devphyid[17];
    int termid;// 终端ID
    // 设备终端号
    uint8 termno[8];
    // 是否加载sam卡成功
    uint8 load_sam;
    // 远程服务器配置
    p16_remote_conf remote[REMOTE_END];
    // service
    char service_path[256];
    // 终端流水号
    uint32 termseqno;
    // 签到操作员ID
    uint32 operid;
    // 签到操作员号
    char opercode[11];
    // 签到操作员名
    char opername[61];
    // 签到后授权号
    uint32 authcode;
    // 签到后批次号
    uint32 batchno;
    // 签到标志
    uint8 login_flag;
    uint32 login_time_tick;
    // 当前时间
    char current_datetime[15];
    // 主显示屏指针
    dis_map* main_disp;
    // 打印机指针
    dis_map* printer_disp;
    // 状态栏指针
    dis_map* status_disp;
    // RF设备句柄
    int32 rfdev;
    // SAM 设备句柄
    int32 samdev;
    // SAM 卡座号
    uint8 samport;
    // 小票联
    uint8 printer_pages;
    int app_running;
    // 最后一次交易是否超时
    int last_trans_timeout;
    // 当先系统配置版本号
    int cfgverno;
    // 工作模式, 0 - 充值机模式， 1 - 补助机模式
    int work_mode;
    // 心跳时间间隔，单位分
    int heartbeat_minutes;
    // 签到后的 check_num
    char check_num[21];
} p16_context;

typedef struct
{
    uint8 cardtype;
    // 物理卡号,二进制
    uint8 cardphyid[8];
    // 交易卡号
    uint32 cardno;
    // 卡状态
    uint8 cardstatus;
    // 卡版本号
    uint8 cardverno[7];
    // 卡交易前余额
    uint32 cardbefbal;
    // 卡交易前消费次数
    uint16 paycnt;
    // 卡交易前充值次数
    uint16 dpscnt;
    // 充值初始化随机数
    uint8 random_num[4];
    // 实际充值金额
    uint32 dpsamt;
    // MAC1
    uint8 mac1[4];
    // MAC2
    uint8 mac2[4];
    // TAC
    uint8 tac[4];
    // 卡收费类别
    uint8 feetype;
    // 有效期
    uint8 expiredate[4];
    // 卡缓存
    uint8 m1_card_buffer[16];
	// 新添加
	uint8 sam_seqno[4];//终端交易号
	uint8 alg_ver;//一个字节：算法标志    
	uint8 key_verno;//一个字节:密钥版本号   
	uint8 card_limit_amt[3];//三个字节:透资限额      
    uint32 payamt; // 实际消费金额

    char hostdate[9];
    char hosttime[7];
    char termdate[9];
    char termtime[7];
    char refno[21];
} p16_card_context;



extern p16_context p16pos;
extern p16_card_context p16card;

#define MAIN_DISP p16pos.main_disp
#define PRINTER p16pos.printer_disp
#define STATUSBAR p16pos.status_disp

#define RFDEV p16pos.rfdev
#define SAMDEV p16pos.samdev

#define SECONDS_ONE_DAY (60*60*24)

#endif // _SW_P16_CONFIG_H_

