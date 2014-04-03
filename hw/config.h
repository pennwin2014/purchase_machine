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

//tcpһ���Է��͵�������ˮ����
#define MAX_TCP_TRANSDTL_CNT 20 
#define MAX_RECV_BLACKLIST_CNT 100


// �Ƿ�֧�� SAM ��
#define ENABLE_SAM 1
// �Ƿ�֧�ִ�ӡ��
#define ENABLE_PRINTER 1
// �Ƿ�֧��GPRS
#define ENABLE_GPRS 1
// �Ƿ�֧��CDMA
#define ENABLE_CDMA 1
// �Ƿ�֧��MODEM
#define ENABLE_MODEM 0

// �Ƿ�ʹ�� M1 ��
#define CARDTYPE_M1
// �Ƿ�ʹ�� CPU ��
#define CRADTYPE_CPU

// ���� CPU Ĭ�� PIN
#define CPU_DEFAULT_PIN "000000"

#define DEBUG_LOG 1

#define DEFAULT_DEVPHYID_LEN 8

#define DEFAULT_HZ_FONT ST_32DZ
#define DEFAULT_ASC_FONT ASC_32DZ

#define INPUT_HZ_FONT ST_24DZ
#define INPUT_ASC_FONT ASC_24DZ

#define REMOTE_WIRE 0 // ��������
#define REMOTE_GPRS 1 // GPRS ����
#define REMOTE_CDMA 2 // CDMA ����
#define REMOTE_MODEM 3 // MODEM ����
#define REMOTE_COM  4 // COM ��
#define REMOTE_WIFI 5 // ��������
#define REMOTE_END 6

// ������ģʽ�¹̶�����ԱID
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
    // ǩ�����session key
    char session_key[65];
    uint8 device_key[16];
    // ����·��
    char base_dir[1024];
    // �豸����ID
    char devphyid[17];
    int termid;// �ն�ID
    // �豸�ն˺�
    uint8 termno[8];
    // �Ƿ����sam���ɹ�
    uint8 load_sam;
    // Զ�̷���������
    p16_remote_conf remote[REMOTE_END];
    // service
    char service_path[256];
    // �ն���ˮ��
    uint32 termseqno;
    // ǩ������ԱID
    uint32 operid;
    // ǩ������Ա��
    char opercode[11];
    // ǩ������Ա��
    char opername[61];
    // ǩ������Ȩ��
    uint32 authcode;
    // ǩ�������κ�
    uint32 batchno;
    // ǩ����־
    uint8 login_flag;
    uint32 login_time_tick;
    // ��ǰʱ��
    char current_datetime[15];
    // ����ʾ��ָ��
    dis_map* main_disp;
    // ��ӡ��ָ��
    dis_map* printer_disp;
    // ״̬��ָ��
    dis_map* status_disp;
    // RF�豸���
    int32 rfdev;
    // SAM �豸���
    int32 samdev;
    // SAM ������
    uint8 samport;
    // СƱ��
    uint8 printer_pages;
    int app_running;
    // ���һ�ν����Ƿ�ʱ
    int last_trans_timeout;
    // ����ϵͳ���ð汾��
    int cfgverno;
    // ����ģʽ, 0 - ��ֵ��ģʽ�� 1 - ������ģʽ
    int work_mode;
    // ����ʱ��������λ��
    int heartbeat_minutes;
    // ǩ����� check_num
    char check_num[21];
	//pos���ĺ������汾��
	uint8 pos_blacklist_version[6];
} p16_context;

typedef struct
{
    uint8 cardtype;
    // ������,������
    uint8 cardphyid[8];
    // ���׿���
    uint32 cardno;
    // ��״̬
    uint8 cardstatus;
    // ���汾��
    uint8 cardverno[7];
    // ������ǰ���
    uint32 cardbefbal;
    // ������ǰ���Ѵ���
    uint16 paycnt;
    // ������ǰ��ֵ����
    uint16 dpscnt;
    // ��ֵ��ʼ�������
    uint8 random_num[4];
    // ʵ�ʳ�ֵ���
    uint32 dpsamt;
    // MAC1
    uint8 mac1[4];
    // MAC2
    uint8 mac2[4];
    // TAC
    uint8 tac[4];
    // ���շ����
    uint8 feetype;
    // ��Ч��
    uint8 expiredate[4];
    // ������
    uint8 m1_card_buffer[16];
	// �����
	uint8 sam_seqno[4];//�ն˽��׺�
	uint8 alg_ver;//һ���ֽڣ��㷨��־    
	uint8 key_verno;//һ���ֽ�:��Կ�汾��   
	uint8 card_limit_amt[3];//�����ֽ�:͸���޶�      
    uint32 payamt; // ʵ�����ѽ��

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

