/******************************************************************************
* Copyright SupWisdom 2012 all right reserved
* Module: �˵�����
* File: menu.c
* Auth: Cheng Tang<cheng.tang@supwisdom.com>
******************************************************************************/

#include <unistd.h>
#include "config.h"
#include "p16log.h"
#include "menu.h"
#include "deposit.h"
#include "utils.h"
#include "syspara.h"
#include "service.h"
#include "version.h"
#include "allowance.h"


// #define  PIC1            "../res/bmp/mediaplayer.bmp"
// #define  PIC2            "../res/bmp/logoff.bmp"
// #define  PIC3            "../res/bmp/windows.bmp"
// #define  PIC4            "../res/bmp/cogwheels.bmp"
// #define  PIC6            "../res/bmp/Shutdown.bmp"
// #define PIC_1     "../res/bmp/pic3.bmp"
//
#define  PIC1      "../res/icon/Repair.ico"
#define  PIC2      "../res/icon/Info.ico"
#define  PIC3      "../res/icon/View.ico"
#define  PIC4      "../res/icon/Comment.ico"
#define  PIC5      "../res/icon/Wizard.ico"
#define  PIC6      "../res/icon/Exit.ico"
#define PIC_1     "../res/ico/pic3.ico"

typedef void(* menu_func)();
typedef struct
{
    char* menu_desc;
    menu_func func;
} menu_item_t;



static void power_off(void)
{
    lcd_160_clr() ;
    sleep(1) ;
    CLEAR_SCREEN;
    dis_mod_set(MAIN_DISP , HZ_24DZ , ASC_24DZ) ;
    printf_str(MAIN_DISP, 10, 75, "����20���رյ�Դ", 0) ;
    lcd_160_upd() ;
    sleep(1) ;
    system("poweroff\n");
}

static void sys_power_off()
{
    int ret = confirm_dialog("ȷ�Ϲػ���");
    if (ret == 0)
    {
        close_config_db();
        power_off();
        exit(0);
    }
}

static proptey  list_menu_property[] =
{
    // {LST_LST_TYPE ,    LST_NOTILE , 0},
    // {LST_WIDTH ,       90, 0},
    // {LST_STA_STATION ,     40 , 77},
    // {LST_FONT_TYPE ,   HZ_24DZ , ASC_24DZ},
    // {LST_BAR_TYP ,         1 , 0},
    // {LST_ROW_LINE ,    2 , 1},
    // {LST_SEL_MOD ,         1 , 0},
    // {LST_ROW_HIGH ,    26 , 1},
    // {0xffff ,          0 , 0}
    {LST_LST_TYPE , LST_NOTILE, 0}, // LST_NOTILE 0����
    {LST_WIDTH ,  DISP_X_PIXEL - 30, 0}, //���
    {LST_STA_STATION ,  DISP_X_COOR + 3, DISP_Y_COOR + INPUT_HEIGHT + 3}, //��ʼ��ʾ����
    {LST_FONT_TYPE ,  INPUT_HZ_FONT, INPUT_ASC_FONT},//����
    {LST_BAK_COLOR,   WHITE_COLR, 0}, // WHITE_COLR//������ɫ ���ܲ�Ҫ��ֻ�ñ���
    {LST_SEL_COLOR,   BLACK_COLR, 1}, //ѡ������ɫ
    {LST_K12315_EN, 1, 1}, //ʹ�ܼ�
    {LST_FON_COLOR,   BLACK_COLR, 0},   //ûѡ��������ɫ
    {LST_FS_COLOR,    WHITE_COLR, 0}, //��ѡ��������ɫ
    {LST_BAR_TYP ,    1 , 0},//�����������
    {LST_ROW_LINE ,   5 , 1},//������
    {LST_SEL_MOD ,    0 , 0}, //ѡ����ģʽ����
    {LST_ROW_HIGH ,   INPUT_HEIGHT , 1},//�и�
    {LST_FKEY_EN ,  1 , 1}, //��ݼ�ʹ��
    {0xffff ,       0 , 0}

};

#define INPUT_Y_COOR 90
#define INPUT_X_COOR (DISP_X_COOR + (DISP_X_PIXEL - INPUT_WIDTH - 15))

proptey input_one_property[7] =
{
    // {INPUT_HIN_STA ,   2 , 80},
    // {INPUT_RET_STA ,   65, 80},
    // {INPUT_RET_SIZ ,   17 , 85},
    // {INPUT_HIND_FONT ,     HZ_16DZ , ASC_12DZ},
    // {INPUT_MAX_LEN ,   16 , 0},
    // {0xffff ,          0 , 0}
    {INPUT_HIN_STA ,  DISP_X_COOR + 5, INPUT_Y_COOR},
    {INPUT_RET_STA ,  INPUT_X_COOR, INPUT_Y_COOR},
    {INPUT_RET_SIZ ,  INPUT_HEIGHT , INPUT_WIDTH},
    {INPUT_HIND_FONT ,  INPUT_HZ_FONT, INPUT_ASC_FONT},
    {INPUT_BCOR_SET , WHITE_COLR, 0},
    {INPUT_MAX_LEN ,  16 , 0},
    {0xffff ,       0 , 0}
};

#define INPUT_TWO_Y_COOR (INPUT_Y_COOR + INPUT_HEIGHT + CTRL_Y_PAD_PIXEL)

proptey input_two_property[7] =
{
    // {INPUT_HIN_STA ,   2 , 100},
    // {INPUT_RET_STA ,   65, 100},
    // {INPUT_RET_SIZ ,   17 , 85},
    // {INPUT_HIND_FONT ,     HZ_16DZ , ASC_12DZ},
    // {INPUT_MAX_LEN ,   16, 0},
    // {0xffff ,          0 , 0}
    {INPUT_HIN_STA ,  DISP_X_COOR + 5, INPUT_TWO_Y_COOR},
    {INPUT_RET_STA ,  INPUT_X_COOR, INPUT_TWO_Y_COOR},
    {INPUT_RET_SIZ ,  INPUT_HEIGHT , INPUT_WIDTH},
    {INPUT_HIND_FONT ,  INPUT_HZ_FONT, INPUT_ASC_FONT},
    {INPUT_BCOR_SET , WHITE_COLR, 0},
    {INPUT_MAX_LEN ,  16 , 0},
    {0xffff ,       0 , 0}
};

#define INPUT_THREE_Y_COOR (INPUT_Y_COOR + (INPUT_HEIGHT + CTRL_Y_PAD_PIXEL) * 2)

proptey input_three_property[7] =
{
    // {INPUT_HIN_STA ,   2 , 120},
    // {INPUT_RET_STA ,   65, 120},
    // {INPUT_RET_SIZ ,   17 , 85},
    // {INPUT_HIND_FONT ,     HZ_16DZ , ASC_12DZ},
    // {INPUT_MAX_LEN ,   16, 0},
    // {0xffff ,          0 , 0}
    {INPUT_HIN_STA ,  DISP_X_COOR + 5, INPUT_THREE_Y_COOR},
    {INPUT_RET_STA ,  INPUT_X_COOR, INPUT_THREE_Y_COOR},
    {INPUT_RET_SIZ ,  INPUT_HEIGHT , INPUT_WIDTH},
    {INPUT_HIND_FONT ,  INPUT_HZ_FONT, INPUT_ASC_FONT},
    {INPUT_BCOR_SET , WHITE_COLR, 0},
    {INPUT_MAX_LEN ,  16 , 0},
    {0xffff ,       0 , 0}
};


static void easy_menu_loop(menu_item_t* menu, char* desc)
{
    list_dst* menu_ptr;
    menu_ptr = list_init();
    if (NULL == menu_ptr)
    {
        error_exit(3, "���ܴ����˵��ؼ�");
    }
    list_props_set(menu_ptr , list_menu_property);
    int menu_count;
    for (menu_count = 0;; ++menu_count)
    {
        if (NULL == menu[menu_count].menu_desc)
            break;
        list_item_add(menu_ptr, menu[menu_count].menu_desc);
    }

    int32 ret;
    dis_mod_set(MAIN_DISP, INPUT_HZ_FONT, INPUT_ASC_FONT);
    while (1)
    {
        CLEAR_SCREEN;
        printf_str(MAIN_DISP, DISP_X_COOR, DISP_Y_COOR, ">", 1);
        printf_str(MAIN_DISP, DISP_X_COOR + 9, DISP_Y_COOR, desc, 1);
        line(MAIN_DISP , DISP_X_COOR, DISP_X_COOR + 28, DISP_X_COOR + 140, DISP_X_COOR + 28, 1);
        line(MAIN_DISP , DISP_X_COOR, DISP_X_COOR + 30, DISP_X_COOR + 140, DISP_X_COOR + 30, 1);
        list_show(MAIN_DISP, menu_ptr);
        lcd_160_upd();

        ret = list_key_manege(MAIN_DISP, menu_ptr, lcd_160_upd);
        if (ret < 0)
        {
            list_exit(menu_ptr) ;
            return ;
        }
        if (ret < menu_count)
        {
            menu[ret].func();
            dis_mod_set(MAIN_DISP, INPUT_HZ_FONT, INPUT_ASC_FONT);
        }
    }

}

static int8 check_admin_pswd()
{
    CLEAR_SCREEN;
    int ret;
    char admin_pswd[64] = {0};
    ret = SAFE_GET_CONFIG("sys.password", admin_pswd);
    if (ret)
    {
        disp_msg("���ܻ�ȡ��������", 5);
        return -1;
    }

    input_dst* pswd_input;
    pswd_input = input_init(PASSWORD_MOD);
    input_props_set(pswd_input, input_one_property);
    input_property_set(pswd_input, INPUT_HIND, "����");
    input_property_set(pswd_input, INPUT_MAX_LEN, 8);
    input_dis(MAIN_DISP, pswd_input);
    printf_str(MAIN_DISP, DISP_X_COOR, DISP_Y_COOR, "���������Ա����", 1);

    lcd_160_upd();

    ret = input_key_manege(MAIN_DISP, pswd_input , lcd_160_upd);
    if (ret >= 0)
    {
        char pswd[9] = {0};
        input_str_get(pswd_input, pswd);
        if (strcmp(pswd, admin_pswd) == 0)
            ret = 0;
        else
        {
            disp_msg("�������", 10);
            ret = -1;
        }
    }
    input_destory(pswd_input);
    return ret;

}

static void sys_cfg_set_local_network()
{
    CLEAR_SCREEN;
    const uint32 input_length = 190;
    input_dst* ip_input, *net_input, *gw_input;
    ip_input = input_init(FREE_MOD);
    net_input = input_init(FREE_MOD);
    gw_input = input_init(FREE_MOD);

    printf_str(MAIN_DISP, DISP_X_COOR, DISP_Y_COOR, "����IP��Ϣ", 1);
    input_props_set(ip_input, input_one_property);
    input_property_set(ip_input, INPUT_HIND, "IP");
    input_property_set(ip_input, INPUT_MAX_LEN, 15);
    input_property_set(ip_input, INPUT_RET_STA, 150, INPUT_Y_COOR);
    input_property_set(ip_input, INPUT_RET_SIZ, INPUT_HEIGHT, input_length);

    input_props_set(net_input, input_two_property);
    input_property_set(net_input, INPUT_HIND, "����");
    input_property_set(net_input, INPUT_MAX_LEN, 15);
    input_property_set(net_input, INPUT_RET_STA, 150, INPUT_TWO_Y_COOR);
    input_property_set(net_input, INPUT_RET_SIZ, INPUT_HEIGHT, input_length);

    input_props_set(gw_input, input_three_property);
    input_property_set(gw_input, INPUT_HIND, "����");
    input_property_set(gw_input, INPUT_MAX_LEN, 15);
    input_property_set(gw_input, INPUT_RET_STA, 150, INPUT_THREE_Y_COOR);
    input_property_set(gw_input, INPUT_RET_SIZ, INPUT_HEIGHT, input_length);

    input_dis(MAIN_DISP, ip_input);
    input_dis(MAIN_DISP, net_input);
    input_dis(MAIN_DISP, gw_input);

    lcd_160_upd();

    input_key_manege(MAIN_DISP, ip_input, lcd_160_upd);
    input_key_manege(MAIN_DISP, net_input, lcd_160_upd);
    input_key_manege(MAIN_DISP, gw_input, lcd_160_upd);

    char ip[24] = {0};
    char netmask[24] = {0};
    char gw[24] = {0};

    input_str_get(ip_input, ip);
    input_str_get(net_input, netmask);
    input_str_get(gw_input, gw);

    input_destory(ip_input);
    input_destory(net_input);
    input_destory(gw_input);

    if (check_ip_valid(ip)
            || check_ip_valid(netmask)
            || check_ip_valid(gw))
    {
        disp_msg("�����ʽ����", 10);
        return;
    }

    if (set_local_ip(ip, netmask, gw))
    {
        disp_msg("�޸�IPʧ��", 10);
    }
    else
    {
        update_config("dev.ip", ip);
        update_config("dev.netmask", netmask);
        update_config("dev.gateway", gw);
        disp_msg("����IP�ɹ�", 5);
    }

}
static void sys_cfg_set_agent()
{
    CLEAR_SCREEN;
    const uint32 input_length = 190;
    input_dst* agent_input, *agent_port_input;
    agent_input = input_init(FREE_MOD);
    agent_port_input = input_init(FREE_MOD);

    input_props_set(agent_input, input_one_property);
    input_property_set(agent_input, INPUT_HIND, "ǰ��IP");
    input_property_set(agent_input, INPUT_MAX_LEN, 15);
    input_property_set(agent_input, INPUT_RET_STA, 176, INPUT_Y_COOR);
    input_property_set(agent_input, INPUT_RET_SIZ, INPUT_HEIGHT, input_length);

    input_props_set(agent_port_input, input_two_property);
    input_property_set(agent_port_input, INPUT_HIND, "ǰ�ö˿�");
    input_property_set(agent_port_input, INPUT_MAX_LEN, 5);
    input_dis(MAIN_DISP, agent_input);
    input_dis(MAIN_DISP, agent_port_input);

    printf_str(MAIN_DISP, DISP_X_COOR, DISP_Y_COOR, "����ǰ�õ�ַ", 1);

    lcd_160_upd();

    input_key_manege(MAIN_DISP, agent_input , lcd_160_upd);
    input_key_manege(MAIN_DISP, agent_port_input , lcd_160_upd);

    char agent_ip[24] = {0};

    input_str_get(agent_input, agent_ip);
    uint32 agent_port = input_val_get(agent_port_input) / 100;

    input_destory(agent_input);
    input_destory(agent_port_input);

    if (check_ip_valid(agent_ip)
            || agent_port < 80 || agent_port > 65535)
    {
        LOG((LOG_DEBUG, "ǰ�û�IP: %s:%d", agent_ip, agent_port));
        disp_msg("��ַ��ʽ����", 10);
        return;
    }

    if (update_config("svc.remotename", agent_ip)
            || update_config_int("svc.remoteport", agent_port))
    {
        disp_msg("����ǰ����Ϣʧ��", 10);
        return;
    }
    SAFE_STR_CPY(p16pos.remote[REMOTE_WIRE].u.host.host_name, agent_ip);
    p16pos.remote[REMOTE_WIRE].u.host.host_port = agent_port;
    setup_svc_url();

    disp_msg("�޸�ǰ����Ϣ�ɹ�", 5);
}

static void sys_cfg_set_dns()
{
    CLEAR_SCREEN;
    input_dst* dns_input;
    dns_input = input_init(FREE_MOD);

    input_props_set(dns_input, input_one_property);
    input_property_set(dns_input, INPUT_HIND, "DNS IP");
    input_dis(MAIN_DISP, dns_input);

    printf_str(MAIN_DISP, DISP_X_COOR, DISP_Y_COOR, "����DNS��ַ", 1);

    lcd_160_upd();

    input_key_manege(MAIN_DISP, dns_input , lcd_160_upd);

    char dns[32] = {0};

    input_str_get(dns_input, dns);

    input_destory(dns_input);

    if (check_ip_valid(dns))
    {
        disp_msg("��ַ��ʽ����", 10);
        return;
    }
    char cmd[128];
    sprintf(cmd, "echo \"nameserver %s\" > /etc/resolv.conf", dns);
    system(cmd);
    disp_msg("�޸�DNS�ɹ�", 5);

}

static void sys_cfg_set_network()
{
    if (check_admin_pswd())
    {
        return;
    }
    menu_item_t menu[] =
    {
        {"1.����IP����", &sys_cfg_set_local_network},
        {"2.ǰ��IP����", &sys_cfg_set_agent},
        {"3.DNS����", &sys_cfg_set_dns},
        {NULL, NULL},
    };

    easy_menu_loop(menu, "�������");
}
static void sys_cfg_set_admin()
{
    CLEAR_SCREEN;
    input_dst* pswd1, *pswd2;
    pswd1 = input_init(PASSWORD_MOD);
    pswd2 = input_init(PASSWORD_MOD);

    input_props_set(pswd1, input_one_property);
    input_property_set(pswd1, INPUT_HIND, "����");
    input_property_set(pswd1, INPUT_MAX_LEN, 8);
    input_props_set(pswd2, input_two_property);
    input_property_set(pswd2, INPUT_HIND, "ȷ��");
    input_property_set(pswd2, INPUT_MAX_LEN, 8);

    input_dis(MAIN_DISP, pswd1);
    input_dis(MAIN_DISP, pswd2);

    lcd_160_upd();

    int ret;
    ret = input_key_manege(MAIN_DISP, pswd1 , lcd_160_upd);
    if (ret < 0)
        goto L_END;

    ret = input_key_manege(MAIN_DISP, pswd2 , lcd_160_upd);
    if (ret < 0)
        goto L_END;

    char pswd1_str[9] = {0};
    char pswd2_str[9] = {0};
    input_str_get(pswd1, pswd1_str);
    input_str_get(pswd2, pswd2_str);


    if (strcmp(pswd1_str, pswd2_str) != 0)
    {
        disp_msg("�������벻��", 10);
    }
    else if (strlen(pswd1_str) < 6 || strlen(pswd1_str) > 8)
    {
        disp_msg("���볤�Ȳ���", 10);
    }
    else if (update_config("sys.password", pswd1_str))
    {
        disp_msg("�޸Ĳ���ʧ��", 10);
    }
    else
    {
        disp_msg("�޸�����ɹ�", 5);
    }
L_END:
    input_destory(pswd1);
    input_destory(pswd2);
}

static void sys_cfg_set_time()
{
    CLEAR_SCREEN;
    input_dst* date_input, *time_input;
    date_input = input_init(FREE_MOD);
    time_input = input_init(FREE_MOD);

    input_props_set(date_input, input_one_property);
    input_property_set(date_input, INPUT_HIND, "����");
    input_property_set(date_input, INPUT_MAX_LEN, 8);
    input_props_set(time_input, input_two_property);
    input_property_set(time_input, INPUT_HIND, "ʱ��");
    input_property_set(time_input, INPUT_MAX_LEN, 6);
    input_dis(MAIN_DISP, date_input);
    input_dis(MAIN_DISP, time_input);

    printf_str(MAIN_DISP, DISP_X_COOR, DISP_Y_COOR, "��ʽYYYYMMDD HHMMSS", 1);

    lcd_160_upd();

    input_key_manege(MAIN_DISP, date_input , lcd_160_upd);
    input_key_manege(MAIN_DISP, time_input , lcd_160_upd);

    char date_buf[24] = {0};
    char time_buf[24] = {0};

    input_str_get(date_input, date_buf);
    input_str_get(time_input, time_buf);

    input_destory(date_input);
    input_destory(time_input);

    char input_buf[24] = {0};
    char output_buf[32] = {0};

    strcpy(input_buf, date_buf);
    strcat(input_buf, time_buf);

    if (strlen(input_buf) != 14
            || convert_datetime(input_buf, "%Y%m%d%H%M%S", "%Y-%m-%d%H:%M:%S",
                                output_buf, sizeof(output_buf) - 1) != 0)
    {
        disp_msg("ʱ���ʽ����", 8);
    }
    else
    {
        char cmd[128];
        strcpy(cmd, "date -D %Y-%m-%d%H:%M:%S -s ");
        strcat(cmd, output_buf);
        system(cmd);
        system("hwclock -w");
        disp_msg("�޸�ʱ��ɹ�", 5);
    }
}
static void sys_cfg_set_devphyid()
{
    CLEAR_SCREEN;
    input_dst* devphyid_input;
    devphyid_input = input_init(FREE_MOD);

    input_props_set(devphyid_input, input_one_property);
    input_property_set(devphyid_input, INPUT_HIND, "�ն˱��");
    input_property_set(devphyid_input, INPUT_MAX_LEN, 8);
    input_dis(MAIN_DISP, devphyid_input);

    printf_str(MAIN_DISP, DISP_X_COOR, DISP_Y_COOR, "����8λ�ն˱��", 1);

    lcd_160_upd();

    input_key_manege(MAIN_DISP, devphyid_input , lcd_160_upd);

    char devphyid_buf[24] = {0};

    input_str_get(devphyid_input, devphyid_buf);

    input_destory(devphyid_input);

    if (strlen(devphyid_buf) != 8)
    {
        disp_msg("�ն˺Ÿ�ʽ����", 10);
        return;
    }

    if (update_config("dev.devphyid", devphyid_buf))
    {
        disp_msg("�����ն˺�ʧ��", 10);
        return;
    }
    strcpy(p16pos.devphyid, devphyid_buf);
    LOG((LOG_DEBUG, "�ն˺�%s", p16pos.devphyid));

    disp_msg("�޸��ն˺ųɹ�", 5);
}

static void sys_cf_set_printerpages()
{
    CLEAR_SCREEN;
    input_dst* pages_input;
    pages_input = input_init(FREE_MOD);

    input_props_set(pages_input, input_one_property);
    input_property_set(pages_input, INPUT_HIND, "СƱ����");
    input_property_set(pages_input, INPUT_MAX_LEN, 1);
    input_dis(MAIN_DISP, pages_input);

    printf_str(MAIN_DISP, DISP_X_COOR, DISP_Y_COOR, "����0-3һλ����", 1);

    lcd_160_upd();

    int ret = input_key_manege(MAIN_DISP, pages_input , lcd_160_upd);
    if (ret < 0)
    {
        input_destory(pages_input);
        return;
    }

    char pages_buf[24] = {0};

    input_str_get(pages_input, pages_buf);


    input_destory(pages_input);

    int pages = atoi(pages_buf);
    if (pages < 0 || pages > 3)
    {
        disp_msg("СƱ������Χ0~3", 10);
        return;
    }

    if (update_config_int("sys.printerpages", pages))
    {
        disp_msg("����СƱ����ʧ��", 10);
        return;
    }
    p16pos.printer_pages = (uint8)pages;

    disp_msg("�޸�СƱ�����ɹ�", 5);
}
static void sys_cfg_clear_transdtl()
{
    int ret = confirm_dialog("ȷ�������ˮ?");
    if (ret != 0)
        return;
    ret = trans_clear_dtl();
    if (!ret)
    {
        disp_msg("�����ˮ�ɹ�", 10);
    }
    else
    {
        disp_msg("�����ˮʧ��\n ����ϵ����Ա", 10);
    }
}

static void sys_cfg_workmode()
{
    CLEAR_SCREEN;
    input_dst* workmode_input;
    workmode_input = input_init(FREE_MOD);

    input_props_set(workmode_input, input_one_property);
    input_property_set(workmode_input, INPUT_HIND, "����ģʽ");
    input_property_set(workmode_input, INPUT_MAX_LEN, 1);
    input_dis(MAIN_DISP, workmode_input);

    printf_str(MAIN_DISP, DISP_X_COOR, DISP_Y_COOR, "0-��ֵ 1-����", 1);

    lcd_160_upd();

    int ret = input_key_manege(MAIN_DISP, workmode_input , lcd_160_upd);
    if (ret < 0)
    {
        input_destory(workmode_input);
        return;
    }

    char input_buf[24] = {0};

    input_str_get(workmode_input, input_buf);

    input_destory(workmode_input);

    int mode = atoi(input_buf);
    if (mode != 0 && mode != 1)
    {
        disp_msg("�������ֵ����ȷ", 10);
        return;
    }

    if (update_config_int("sys.workmode", mode))
    {
        disp_msg("���¹���ģʽʧ��", 10);
        return;
    }
    p16pos.work_mode = mode;

    disp_msg("�޸ĳɹ������ֶ�����", 5);
}

static void sys_cfg_heartbeat()
{
    CLEAR_SCREEN;
    input_dst* heartbeat_input;
    heartbeat_input = input_init(FREE_MOD);

    input_props_set(heartbeat_input, input_one_property);
    input_property_set(heartbeat_input, INPUT_HIND, "��������");
    input_property_set(heartbeat_input, INPUT_MAX_LEN, 1);
    input_dis(MAIN_DISP, heartbeat_input);

    printf_str(MAIN_DISP, DISP_X_COOR, DISP_Y_COOR, "��λ�֣�0~10", 1);

    lcd_160_upd();

    int ret = input_key_manege(MAIN_DISP, heartbeat_input , lcd_160_upd);
    if (ret < 0)
    {
        input_destory(heartbeat_input);
        return;
    }

    char input_buf[24] = {0};

    input_str_get(heartbeat_input, input_buf);

    input_destory(heartbeat_input);

    int heartbeat = atoi(input_buf);
    if (heartbeat < 0 || heartbeat > 10)
    {
        disp_msg("�������ֵ����ȷ", 10);
        return;
    }

    if (update_config_int("sys.heartbeatminutes", heartbeat))
    {
        disp_msg("���¹���ģʽʧ��", 10);
        return;
    }
    p16pos.heartbeat_minutes = heartbeat;

    disp_msg("�޸ĳɹ������ֶ�����", 5);

}

static void sys_cfg_set_sys_para()
{
    if (check_admin_pswd())
    {
        return;
    }
    CLEAR_SCREEN;
    menu_item_t menu[] =
    {
        {"1.����Ա����", &sys_cfg_set_admin},
        {"2.ʱ�Ӳ���", &sys_cfg_set_time},
        {"3.�ն˱��", &sys_cfg_set_devphyid},
        {"4.СƱ��ӡ", &sys_cf_set_printerpages},
        {"5.�����ˮ", &sys_cfg_clear_transdtl},
        {"6.����ģʽ", &sys_cfg_workmode},
        {"7.��������", &sys_cfg_heartbeat},
        {NULL, NULL},
    };
    easy_menu_loop(menu, "ϵͳ����");
}
static void sys_cfg_rom_update()
{
    CLEAR_SCREEN;
    disp_msg("δʵ�ֹ���", 5);
}
static void sys_cfg_version()
{
    CLEAR_SCREEN;
    int idx = 0;
    int lineno = 0;
    int maxlineno = 0;
    struct
    {
        int lineno;
        char info[64];
    } dev_msg[18];
    dev_msg[idx].lineno = maxlineno;
    sprintf(dev_msg[idx++].info, "�汾��V%s", P16POS_VERSION);
    dev_msg[idx].lineno = maxlineno;
    strcpy(dev_msg[idx++].info, "��Ȩ����ά��Ϣ");
    dev_msg[idx].lineno = maxlineno;
    sprintf(dev_msg[idx++].info, "�ն˺ţ�%s", p16pos.devphyid);
    dev_msg[idx].lineno = maxlineno;
    sprintf(dev_msg[idx++].info, "�����汾��%d", p16pos.cfgverno);
    ++maxlineno;

    char temp[64];

    dev_msg[idx].lineno = maxlineno;
    SAFE_GET_CONFIG("dev.ip", temp);
    sprintf(dev_msg[idx++].info, "IP: %s", temp);
    dev_msg[idx].lineno = maxlineno;
    SAFE_GET_CONFIG("dev.netmask", temp);
    sprintf(dev_msg[idx++].info, "netmask: %s", temp);
    dev_msg[idx].lineno = maxlineno;
    SAFE_GET_CONFIG("dev.gateway", temp);
    sprintf(dev_msg[idx++].info, "����: %s", temp);
    ++maxlineno;

    dev_msg[idx].lineno = maxlineno;
    sprintf(dev_msg[idx++].info, "ǰ��: %s", p16pos.remote[REMOTE_WIRE].u.host.host_name);
    dev_msg[idx].lineno = maxlineno;
    sprintf(dev_msg[idx++].info, "ǰ�ö˿�: %d", p16pos.remote[REMOTE_WIRE].u.host.host_port);
    dev_msg[idx].lineno = maxlineno;
    sprintf(dev_msg[idx++].info, "СƱ����%d��", p16pos.printer_pages);
    ++maxlineno;

    int ret, i;
    uint8 key, y_coor;
    while (1)
    {
        CLEAR_SCREEN;
        y_coor = DISP_Y_COOR;
        char tip[64];
        sprintf(tip, "** ҳ��: %d/%d **", lineno + 1, maxlineno);
        printf_str(MAIN_DISP, DISP_X_COOR, y_coor, tip, 1);
        y_coor += 26;
        line(MAIN_DISP, DISP_X_COOR, y_coor, DISP_X_COOR + DISP_X_PIXEL, y_coor, 1);
        y_coor += 4;
        for (i = 0; i < idx; ++i)
        {
            if (dev_msg[i].lineno == lineno)
            {
                printf_str(MAIN_DISP, DISP_X_COOR, y_coor, dev_msg[i].info, 1);
                y_coor += 26;
            }
        }
        lcd_160_upd();
        ret = get_key(&key, BLOCK_MOD, -1);
        if (ret < 0)
            continue;
        if (KEY_UP == key)
        {
            if (lineno > 0)
                --lineno;
        }
        else if (KEY_DN == key)
        {
            if (lineno < maxlineno - 1)
            {
                ++lineno;
            }
        }
        else if (ESC_KEY == key)
        {
            return;
        }
    }
}
static void sys_cfg_reset()
{
    if (check_admin_pswd())
    {
        return;
    }

    disp_msg("׼���ָ���������", 0);
    if (syspara_reset())
    {
        disp_msg("�ָ��������óɹ�", 3);
    }
    else
    {
        disp_msg("�ָ���������ʧ��", 3);
    }
}
static void sys_config()
{
    menu_item_t menu[] =
    {
        {"1.�������", &sys_cfg_set_network},
        {"2.ϵͳ����", &sys_cfg_set_sys_para},
        {"3.��������", &sys_cfg_rom_update},
        {"4.�ն���Ϣ", &sys_cfg_version},
        {"5.�ָ���������", &sys_cfg_reset},
        {NULL, NULL},
    };

    easy_menu_loop(menu, "ϵͳ����");
}
static void oper_logout()
{
    if (p16pos.login_flag == 1)
    {
        int ret;
        ret = confirm_dialog("ȷ��ǩ������Ա��");
        if (ret < 0)
            return;
        p16pos.login_flag = 0;
        p16pos.operid = 0;
        p16pos.authcode = 0;
        p16pos.batchno = 0;
        disp_msg("����Աǩ���ɹ�", 6);
    }
    else
    {
        disp_msg("����Ҫǩ��", 5);
    }
}

static void do_statistic_oper_by_day(const char* today)
{
    if (p16pos.login_flag != 1)
    {
        disp_msg("�����Ա�ȵ�¼", 10);
        return;
    }

    oper_trans_t trans;
    memset(&trans, 0, sizeof trans);
    int ret = oper_get_trans(today, p16pos.operid, &trans);
    if (ret)
    {
        disp_msg("��ѯʧ��\n����ϵ����Ա", 10);
        return;
    }
    char tip[64];
    CLEAR_SCREEN;

    uint8 y_coor = DISP_Y_COOR;
    sprintf(tip, "����Ա %s", p16pos.opername);
    printf_str(MAIN_DISP, DISP_X_COOR, y_coor, tip, 1);
    y_coor += 26;
    sprintf(tip, "ͳ������ %s", today);
    printf_str(MAIN_DISP, DISP_X_COOR, y_coor, tip, 1);
    y_coor += 26;
    line(MAIN_DISP, DISP_X_COOR, y_coor, DISP_X_COOR + 200, y_coor, 1);
    line(MAIN_DISP, DISP_X_COOR, y_coor + 1, DISP_X_COOR + 200, y_coor + 1, 1);
    y_coor += 6;

    int index = 0;
    uint8 key;
    dis_mod_set(MAIN_DISP, DEFAULT_HZ_FONT, DEFAULT_ASC_FONT);
    while (1)
    {
        // box(MAIN_DISP, 0, y_coor, 160, 160, 0);
        CLEAR(MAIN_DISP, DISP_X_COOR, y_coor, DISP_X_COOR + DISP_X_PIXEL, DISP_Y_PIXEL);
        if (index == 0)
        {
            sprintf(tip, "�ɹ� %d��", trans.success_cnt);
            printf_str(MAIN_DISP, DISP_X_COOR, y_coor, tip, 1);
            sprintf(tip, " %0.2fԪ", trans.success_amt / 100.0);
            printf_str(MAIN_DISP, DISP_X_COOR, y_coor + INPUT_HEIGHT, tip, 1);
        }
        else if (index == 1)
        {
            sprintf(tip, "ʧ�� %d��", trans.total_cnt - trans.success_cnt);
            printf_str(MAIN_DISP, DISP_X_COOR, y_coor, tip, 1);
            sprintf(tip, " %0.2fԪ", (trans.total_amt - trans.success_amt) / 100.0);
            printf_str(MAIN_DISP, DISP_X_COOR, y_coor + INPUT_HEIGHT, tip, 1);
        }
        else
        {
            sprintf(tip, "���� %d��", trans.total_cnt);
            printf_str(MAIN_DISP, DISP_X_COOR, y_coor, tip, 1);
            sprintf(tip, " %0.2fԪ", (trans.total_amt) / 100.0);
            printf_str(MAIN_DISP, DISP_X_COOR, y_coor + INPUT_HEIGHT, tip, 1);
        }
        lcd_160_upd();
        ret = get_key(&key, BLOCK_MOD, -1);
        if (ret < 0)
            continue;
        if (ESC_KEY == key)
        {
            return;
        }
        else if (KEY_UP == key)
        {
            --index;
            if (index < 0) index = 0;
        }
        else if (KEY_DN == key)
        {
            ++index;
            if (index > 2) index = 2;
        }
    }

}
static void statistic_oper_today()
{
    char today[17] = {0};
    SAFE_GET_DATETIME("%Y%m%d", today);
    do_statistic_oper_by_day(today);
}
static void statistic_oper_yesterday()
{
    char yesterday[17] = {0};
    get_datetime_offset(-SECONDS_ONE_DAY, "%Y%m%d", yesterday);
    do_statistic_oper_by_day(yesterday);
}
static void statistic_main()
{
    menu_item_t menu[] =
    {
        {"1.���ջ���", &statistic_oper_today},
        {"2.���ջ���", &statistic_oper_yesterday},
        {NULL, NULL},
    };

    easy_menu_loop(menu, "����Ա����");

}

static void oper_login()
{
    if (p16pos.login_flag == 1)
    {
        disp_msg("��ǩ��,����ǩ��", 10);
        return;
    }
    if (!check_oper_login())
    {
        disp_msg("ǩ���ɹ�", 5);
    }
}

static void oper_printer_last()
{
    int ret;
    p16_transdtl_t transdtl;
    memset(&transdtl, 0, sizeof transdtl);
    ret = trans_get_last_succ(&transdtl, p16pos.operid);
    if (ret == 0)
    {
        disp_msg("����ˮ", 10);
        return;
    }
    else if (ret < 0)
    {
        disp_msg("��ѯ��ˮʧ��,����ϵ����Ա", 10);
    }
    else
    {
        ret = pos_printer(&transdtl, 1, 1);
        if (ret == 0)
        {
            disp_msg("��ӡ�ɹ�", 5);
        }
    }
}
static void oper_printer_by_devseqno()
{
    CLEAR_SCREEN;
    input_dst* devseqno_input;
    devseqno_input = input_init(FREE_MOD);

    input_props_set(devseqno_input, input_one_property);
    input_property_set(devseqno_input, INPUT_HIND, "��ˮ��");
    input_property_set(devseqno_input, INPUT_MAX_LEN, 10);
    input_dis(MAIN_DISP, devseqno_input);

    printf_str(MAIN_DISP, DISP_X_COOR, DISP_Y_COOR, "�����ն���ˮ��", 1);

    lcd_160_upd();

    input_key_manege(MAIN_DISP, devseqno_input , lcd_160_upd);

    char seqno_buf[24] = {0};

    input_str_get(devseqno_input, seqno_buf);

    input_destory(devseqno_input);

    int devseqno = atoi(seqno_buf);

    int ret;
    p16_transdtl_t transdtl;
    memset(&transdtl, 0, sizeof transdtl);
    ret = trans_get_oper_devseqno(&transdtl, p16pos.operid, devseqno);
    if (ret == 0)
    {
        disp_msg("����ˮ", 10);
        return;
    }
    else if (ret < 0)
    {
        disp_msg("��ѯ��ˮʧ��,����ϵ����Ա", 10);
    }
    else
    {
        if (transdtl.transflag == TRANS_SYS_FAILED || transdtl.transflag == TRANS_INIT)
        {
            disp_msg("��Ч��ˮ���ܴ�ӡ", 3);
            return;
        }
        ret = pos_printer(&transdtl, 1, 1);
        if (ret == 0)
        {
            disp_msg("��ӡ�ɹ�", 5);
        }
    }
}
static void oper_pos_printer()
{
    if (p16pos.login_flag != 1)
    {
        disp_msg("�����Ա�ȵ�¼", 10);
        return;
    }
    menu_item_t menu[] =
    {
        {"1.���һ��", &oper_printer_last},
        {"2.����ˮ��", &oper_printer_by_devseqno},
        {NULL, NULL},
    };
    easy_menu_loop(menu, "СƱ��ӡ");
}

static void deposit_menu()
{
    menu_item_t menu[] =
    {
        {"1.�ֽ��ֵ", &deposit_main},
        {"2.����Ա����", &statistic_main},
        {"3.����Աǩ��", &oper_login},
        {"4.СƱ����", &oper_pos_printer},
        {"5.����Աǩ��", &oper_logout},
        {NULL, NULL},
    };

    easy_menu_loop(menu, "�ֽ��ֵ");
}

static proptey  main_menu_property[] =
{
    // {BMP_STAT_SET , 2 , 60},
    // {BMP_SIZE_SET , 36 , 40},
    // {BMP_INTE_SET , 4 , 0},
    // {BMP_CAPMOD_SET, 4 , STA_CENTER},
    // {BMP_CAPEN_SET , 1 , 0},
    // {BMP_CAPSIZ_SET , 160 , 16},
    // {BMP_CAPSTA_SET , 0 , 20},
    // {0xffff , 0 , 0}
    {BMP_STAT_SET ,   DISP_X_COOR + 5 , DISP_Y_COOR + 5},
    {BMP_SIZE_SET ,   64 , 64},
    {BMP_INTE_SET ,   16 , 0},
    {BMP_CAPMOD_SET ,   4 , STA_CENTER},
    {BMP_CAPEN_SET ,  1 , 0},
    {BMP_CAPSIZ_SET , (SCREEN_X_PIXEL - 100) / 2 , INPUT_HEIGHT},
    {BMP_CAPSTA_SET ,   DISP_X_COOR + 10, DISP_Y_COOR + 100},
    {BMP_CAPFON_SET,    ST_32DZ , ST_32DZ},
    {PIC_BAK_SET ,    1 , 1},
    {0xffff ,       0 , 0}
};

void menu_purchase_input()
{
	purchase_main();
}

void init_menu()
{
    struct pic_lst* main_menu_ptr;
    char work_menu_name [64] = {0};
    char* main_menu_table[] =
    {
        PIC4, work_menu_name,
        PIC1, "��������",
        PIC6, "�ػ�",
        NULL, NULL
    };

    if (p16pos.work_mode == 0)
    {
        strcpy(work_menu_name, "�ֽ��ֵ");
    }
    else
    {
        strcpy(work_menu_name, "������ȡ");
    }

    main_menu_ptr = piclist_init();
    if (NULL == main_menu_ptr)
    {
        error_exit(2, "��ʼ����Ļ����ʧ��");
    }
    jpg_display(MAIN_DISP, 0, 0, DESKTOP_JPG) ;  //gepspxs19.jpg //  ltby.jpg
    jpg_stormem(MAIN_DISP, MAIN_DISP->bak_ptr, 0, 0, DESKTOP_JPG);

    piclist_props_set(main_menu_ptr, main_menu_table, main_menu_property);
    piclist_show(p16pos.main_disp, main_menu_ptr);
    // ���˵�ѭ��
    while (1)
    {
        // CLEAR(MAIN_DISP, DISP_X_COOR - 5, DISP_Y_COOR - 5,
        //       DISP_X_COOR + DISP_X_PIXEL + 10, DISP_Y_COOR + DISP_Y_PIXEL + 10);
        CLEAR_SCREEN;
        piclist_show(MAIN_DISP, main_menu_ptr);
        lcd_160_upd();

        int32 ret = piclist_key_manege(MAIN_DISP, main_menu_ptr, lcd_160_upd);
        //LOG( ( LOG_DEBUG, "�˵�������key=%d", ret ) );
        switch (ret)
        {
        case 0:
            if (p16pos.work_mode == 0)
                deposit_menu();
            else
                allowance_menu();
            break;
        case 1:
            sys_config();
            break;
        case 2:
            sys_power_off();
            break;
        default:
            break;
        }
    }
}

