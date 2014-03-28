/******************************************************************************
* Copyright SupWisdom 2012 all right reserved
* Module: 菜单界面
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
    printf_str(MAIN_DISP, 10, 75, "请在20秒后关闭电源", 0) ;
    lcd_160_upd() ;
    sleep(1) ;
    system("poweroff\n");
}

static void sys_power_off()
{
    int ret = confirm_dialog("确认关机吗？");
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
    {LST_LST_TYPE , LST_NOTILE, 0}, // LST_NOTILE 0类型
    {LST_WIDTH ,  DISP_X_PIXEL - 30, 0}, //宽度
    {LST_STA_STATION ,  DISP_X_COOR + 3, DISP_Y_COOR + INPUT_HEIGHT + 3}, //开始显示坐标
    {LST_FONT_TYPE ,  INPUT_HZ_FONT, INPUT_ASC_FONT},//字体
    {LST_BAK_COLOR,   WHITE_COLR, 0}, // WHITE_COLR//背景颜色 不能不要，只用背景
    {LST_SEL_COLOR,   BLACK_COLR, 1}, //选中项颜色
    {LST_K12315_EN, 1, 1}, //使能键
    {LST_FON_COLOR,   BLACK_COLR, 0},   //没选择字体颜色
    {LST_FS_COLOR,    WHITE_COLR, 0}, //被选中字体颜色
    {LST_BAR_TYP ,    1 , 0},//带滚动条与否
    {LST_ROW_LINE ,   5 , 1},//行列数
    {LST_SEL_MOD ,    0 , 0}, //选中行模式设置
    {LST_ROW_HIGH ,   INPUT_HEIGHT , 1},//行高
    {LST_FKEY_EN ,  1 , 1}, //快捷键使能
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
        error_exit(3, "不能创建菜单控件");
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
        disp_msg("不能获取管理密码", 5);
        return -1;
    }

    input_dst* pswd_input;
    pswd_input = input_init(PASSWORD_MOD);
    input_props_set(pswd_input, input_one_property);
    input_property_set(pswd_input, INPUT_HIND, "密码");
    input_property_set(pswd_input, INPUT_MAX_LEN, 8);
    input_dis(MAIN_DISP, pswd_input);
    printf_str(MAIN_DISP, DISP_X_COOR, DISP_Y_COOR, "请输入管理员密码", 1);

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
            disp_msg("密码错误", 10);
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

    printf_str(MAIN_DISP, DISP_X_COOR, DISP_Y_COOR, "输入IP信息", 1);
    input_props_set(ip_input, input_one_property);
    input_property_set(ip_input, INPUT_HIND, "IP");
    input_property_set(ip_input, INPUT_MAX_LEN, 15);
    input_property_set(ip_input, INPUT_RET_STA, 150, INPUT_Y_COOR);
    input_property_set(ip_input, INPUT_RET_SIZ, INPUT_HEIGHT, input_length);

    input_props_set(net_input, input_two_property);
    input_property_set(net_input, INPUT_HIND, "掩码");
    input_property_set(net_input, INPUT_MAX_LEN, 15);
    input_property_set(net_input, INPUT_RET_STA, 150, INPUT_TWO_Y_COOR);
    input_property_set(net_input, INPUT_RET_SIZ, INPUT_HEIGHT, input_length);

    input_props_set(gw_input, input_three_property);
    input_property_set(gw_input, INPUT_HIND, "网关");
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
        disp_msg("输入格式错误", 10);
        return;
    }

    if (set_local_ip(ip, netmask, gw))
    {
        disp_msg("修改IP失败", 10);
    }
    else
    {
        update_config("dev.ip", ip);
        update_config("dev.netmask", netmask);
        update_config("dev.gateway", gw);
        disp_msg("设置IP成功", 5);
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
    input_property_set(agent_input, INPUT_HIND, "前置IP");
    input_property_set(agent_input, INPUT_MAX_LEN, 15);
    input_property_set(agent_input, INPUT_RET_STA, 176, INPUT_Y_COOR);
    input_property_set(agent_input, INPUT_RET_SIZ, INPUT_HEIGHT, input_length);

    input_props_set(agent_port_input, input_two_property);
    input_property_set(agent_port_input, INPUT_HIND, "前置端口");
    input_property_set(agent_port_input, INPUT_MAX_LEN, 5);
    input_dis(MAIN_DISP, agent_input);
    input_dis(MAIN_DISP, agent_port_input);

    printf_str(MAIN_DISP, DISP_X_COOR, DISP_Y_COOR, "输入前置地址", 1);

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
        LOG((LOG_DEBUG, "前置机IP: %s:%d", agent_ip, agent_port));
        disp_msg("地址格式错误", 10);
        return;
    }

    if (update_config("svc.remotename", agent_ip)
            || update_config_int("svc.remoteport", agent_port))
    {
        disp_msg("更新前置信息失败", 10);
        return;
    }
    SAFE_STR_CPY(p16pos.remote[REMOTE_WIRE].u.host.host_name, agent_ip);
    p16pos.remote[REMOTE_WIRE].u.host.host_port = agent_port;
    setup_svc_url();

    disp_msg("修改前置信息成功", 5);
}

static void sys_cfg_set_dns()
{
    CLEAR_SCREEN;
    input_dst* dns_input;
    dns_input = input_init(FREE_MOD);

    input_props_set(dns_input, input_one_property);
    input_property_set(dns_input, INPUT_HIND, "DNS IP");
    input_dis(MAIN_DISP, dns_input);

    printf_str(MAIN_DISP, DISP_X_COOR, DISP_Y_COOR, "输入DNS地址", 1);

    lcd_160_upd();

    input_key_manege(MAIN_DISP, dns_input , lcd_160_upd);

    char dns[32] = {0};

    input_str_get(dns_input, dns);

    input_destory(dns_input);

    if (check_ip_valid(dns))
    {
        disp_msg("地址格式错误", 10);
        return;
    }
    char cmd[128];
    sprintf(cmd, "echo \"nameserver %s\" > /etc/resolv.conf", dns);
    system(cmd);
    disp_msg("修改DNS成功", 5);

}

static void sys_cfg_set_network()
{
    if (check_admin_pswd())
    {
        return;
    }
    menu_item_t menu[] =
    {
        {"1.本机IP设置", &sys_cfg_set_local_network},
        {"2.前置IP设置", &sys_cfg_set_agent},
        {"3.DNS设置", &sys_cfg_set_dns},
        {NULL, NULL},
    };

    easy_menu_loop(menu, "网络参数");
}
static void sys_cfg_set_admin()
{
    CLEAR_SCREEN;
    input_dst* pswd1, *pswd2;
    pswd1 = input_init(PASSWORD_MOD);
    pswd2 = input_init(PASSWORD_MOD);

    input_props_set(pswd1, input_one_property);
    input_property_set(pswd1, INPUT_HIND, "密码");
    input_property_set(pswd1, INPUT_MAX_LEN, 8);
    input_props_set(pswd2, input_two_property);
    input_property_set(pswd2, INPUT_HIND, "确认");
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
        disp_msg("输入密码不符", 10);
    }
    else if (strlen(pswd1_str) < 6 || strlen(pswd1_str) > 8)
    {
        disp_msg("密码长度不符", 10);
    }
    else if (update_config("sys.password", pswd1_str))
    {
        disp_msg("修改参数失败", 10);
    }
    else
    {
        disp_msg("修改密码成功", 5);
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
    input_property_set(date_input, INPUT_HIND, "日期");
    input_property_set(date_input, INPUT_MAX_LEN, 8);
    input_props_set(time_input, input_two_property);
    input_property_set(time_input, INPUT_HIND, "时间");
    input_property_set(time_input, INPUT_MAX_LEN, 6);
    input_dis(MAIN_DISP, date_input);
    input_dis(MAIN_DISP, time_input);

    printf_str(MAIN_DISP, DISP_X_COOR, DISP_Y_COOR, "格式YYYYMMDD HHMMSS", 1);

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
        disp_msg("时间格式错误", 8);
    }
    else
    {
        char cmd[128];
        strcpy(cmd, "date -D %Y-%m-%d%H:%M:%S -s ");
        strcat(cmd, output_buf);
        system(cmd);
        system("hwclock -w");
        disp_msg("修改时间成功", 5);
    }
}
static void sys_cfg_set_devphyid()
{
    CLEAR_SCREEN;
    input_dst* devphyid_input;
    devphyid_input = input_init(FREE_MOD);

    input_props_set(devphyid_input, input_one_property);
    input_property_set(devphyid_input, INPUT_HIND, "终端编号");
    input_property_set(devphyid_input, INPUT_MAX_LEN, 8);
    input_dis(MAIN_DISP, devphyid_input);

    printf_str(MAIN_DISP, DISP_X_COOR, DISP_Y_COOR, "输入8位终端编号", 1);

    lcd_160_upd();

    input_key_manege(MAIN_DISP, devphyid_input , lcd_160_upd);

    char devphyid_buf[24] = {0};

    input_str_get(devphyid_input, devphyid_buf);

    input_destory(devphyid_input);

    if (strlen(devphyid_buf) != 8)
    {
        disp_msg("终端号格式错误", 10);
        return;
    }

    if (update_config("dev.devphyid", devphyid_buf))
    {
        disp_msg("更新终端号失败", 10);
        return;
    }
    strcpy(p16pos.devphyid, devphyid_buf);
    LOG((LOG_DEBUG, "终端号%s", p16pos.devphyid));

    disp_msg("修改终端号成功", 5);
}

static void sys_cf_set_printerpages()
{
    CLEAR_SCREEN;
    input_dst* pages_input;
    pages_input = input_init(FREE_MOD);

    input_props_set(pages_input, input_one_property);
    input_property_set(pages_input, INPUT_HIND, "小票联数");
    input_property_set(pages_input, INPUT_MAX_LEN, 1);
    input_dis(MAIN_DISP, pages_input);

    printf_str(MAIN_DISP, DISP_X_COOR, DISP_Y_COOR, "输入0-3一位数字", 1);

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
        disp_msg("小票联数范围0~3", 10);
        return;
    }

    if (update_config_int("sys.printerpages", pages))
    {
        disp_msg("更新小票联数失败", 10);
        return;
    }
    p16pos.printer_pages = (uint8)pages;

    disp_msg("修改小票联数成功", 5);
}
static void sys_cfg_clear_transdtl()
{
    int ret = confirm_dialog("确认清空流水?");
    if (ret != 0)
        return;
    ret = trans_clear_dtl();
    if (!ret)
    {
        disp_msg("清空流水成功", 10);
    }
    else
    {
        disp_msg("清空流水失败\n 请联系管理员", 10);
    }
}

static void sys_cfg_workmode()
{
    CLEAR_SCREEN;
    input_dst* workmode_input;
    workmode_input = input_init(FREE_MOD);

    input_props_set(workmode_input, input_one_property);
    input_property_set(workmode_input, INPUT_HIND, "工作模式");
    input_property_set(workmode_input, INPUT_MAX_LEN, 1);
    input_dis(MAIN_DISP, workmode_input);

    printf_str(MAIN_DISP, DISP_X_COOR, DISP_Y_COOR, "0-充值 1-补助", 1);

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
        disp_msg("输入参数值不正确", 10);
        return;
    }

    if (update_config_int("sys.workmode", mode))
    {
        disp_msg("更新工作模式失败", 10);
        return;
    }
    p16pos.work_mode = mode;

    disp_msg("修改成功，请手动重启", 5);
}

static void sys_cfg_heartbeat()
{
    CLEAR_SCREEN;
    input_dst* heartbeat_input;
    heartbeat_input = input_init(FREE_MOD);

    input_props_set(heartbeat_input, input_one_property);
    input_property_set(heartbeat_input, INPUT_HIND, "心跳参数");
    input_property_set(heartbeat_input, INPUT_MAX_LEN, 1);
    input_dis(MAIN_DISP, heartbeat_input);

    printf_str(MAIN_DISP, DISP_X_COOR, DISP_Y_COOR, "单位分，0~10", 1);

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
        disp_msg("输入参数值不正确", 10);
        return;
    }

    if (update_config_int("sys.heartbeatminutes", heartbeat))
    {
        disp_msg("更新工作模式失败", 10);
        return;
    }
    p16pos.heartbeat_minutes = heartbeat;

    disp_msg("修改成功，请手动重启", 5);

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
        {"1.管理员密码", &sys_cfg_set_admin},
        {"2.时钟参数", &sys_cfg_set_time},
        {"3.终端编号", &sys_cfg_set_devphyid},
        {"4.小票打印", &sys_cf_set_printerpages},
        {"5.清空流水", &sys_cfg_clear_transdtl},
        {"6.工作模式", &sys_cfg_workmode},
        {"7.心跳参数", &sys_cfg_heartbeat},
        {NULL, NULL},
    };
    easy_menu_loop(menu, "系统参数");
}
static void sys_cfg_rom_update()
{
    CLEAR_SCREEN;
    disp_msg("未实现功能", 5);
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
    sprintf(dev_msg[idx++].info, "版本：V%s", P16POS_VERSION);
    dev_msg[idx].lineno = maxlineno;
    strcpy(dev_msg[idx++].info, "版权：树维信息");
    dev_msg[idx].lineno = maxlineno;
    sprintf(dev_msg[idx++].info, "终端号：%s", p16pos.devphyid);
    dev_msg[idx].lineno = maxlineno;
    sprintf(dev_msg[idx++].info, "参数版本：%d", p16pos.cfgverno);
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
    sprintf(dev_msg[idx++].info, "网关: %s", temp);
    ++maxlineno;

    dev_msg[idx].lineno = maxlineno;
    sprintf(dev_msg[idx++].info, "前置: %s", p16pos.remote[REMOTE_WIRE].u.host.host_name);
    dev_msg[idx].lineno = maxlineno;
    sprintf(dev_msg[idx++].info, "前置端口: %d", p16pos.remote[REMOTE_WIRE].u.host.host_port);
    dev_msg[idx].lineno = maxlineno;
    sprintf(dev_msg[idx++].info, "小票联：%d联", p16pos.printer_pages);
    ++maxlineno;

    int ret, i;
    uint8 key, y_coor;
    while (1)
    {
        CLEAR_SCREEN;
        y_coor = DISP_Y_COOR;
        char tip[64];
        sprintf(tip, "** 页数: %d/%d **", lineno + 1, maxlineno);
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

    disp_msg("准备恢复出厂设置", 0);
    if (syspara_reset())
    {
        disp_msg("恢复出厂设置成功", 3);
    }
    else
    {
        disp_msg("恢复出厂设置失败", 3);
    }
}
static void sys_config()
{
    menu_item_t menu[] =
    {
        {"1.网络参数", &sys_cfg_set_network},
        {"2.系统参数", &sys_cfg_set_sys_para},
        {"3.在线升级", &sys_cfg_rom_update},
        {"4.终端信息", &sys_cfg_version},
        {"5.恢复出厂设置", &sys_cfg_reset},
        {NULL, NULL},
    };

    easy_menu_loop(menu, "系统参数");
}
static void oper_logout()
{
    if (p16pos.login_flag == 1)
    {
        int ret;
        ret = confirm_dialog("确定签出操作员？");
        if (ret < 0)
            return;
        p16pos.login_flag = 0;
        p16pos.operid = 0;
        p16pos.authcode = 0;
        p16pos.batchno = 0;
        disp_msg("操作员签出成功", 6);
    }
    else
    {
        disp_msg("不需要签出", 5);
    }
}

static void do_statistic_oper_by_day(const char* today)
{
    if (p16pos.login_flag != 1)
    {
        disp_msg("请操作员先登录", 10);
        return;
    }

    oper_trans_t trans;
    memset(&trans, 0, sizeof trans);
    int ret = oper_get_trans(today, p16pos.operid, &trans);
    if (ret)
    {
        disp_msg("查询失败\n请联系管理员", 10);
        return;
    }
    char tip[64];
    CLEAR_SCREEN;

    uint8 y_coor = DISP_Y_COOR;
    sprintf(tip, "操作员 %s", p16pos.opername);
    printf_str(MAIN_DISP, DISP_X_COOR, y_coor, tip, 1);
    y_coor += 26;
    sprintf(tip, "统计日期 %s", today);
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
            sprintf(tip, "成功 %d笔", trans.success_cnt);
            printf_str(MAIN_DISP, DISP_X_COOR, y_coor, tip, 1);
            sprintf(tip, " %0.2f元", trans.success_amt / 100.0);
            printf_str(MAIN_DISP, DISP_X_COOR, y_coor + INPUT_HEIGHT, tip, 1);
        }
        else if (index == 1)
        {
            sprintf(tip, "失败 %d笔", trans.total_cnt - trans.success_cnt);
            printf_str(MAIN_DISP, DISP_X_COOR, y_coor, tip, 1);
            sprintf(tip, " %0.2f元", (trans.total_amt - trans.success_amt) / 100.0);
            printf_str(MAIN_DISP, DISP_X_COOR, y_coor + INPUT_HEIGHT, tip, 1);
        }
        else
        {
            sprintf(tip, "总数 %d笔", trans.total_cnt);
            printf_str(MAIN_DISP, DISP_X_COOR, y_coor, tip, 1);
            sprintf(tip, " %0.2f元", (trans.total_amt) / 100.0);
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
        {"1.当日汇总", &statistic_oper_today},
        {"2.昨日汇总", &statistic_oper_yesterday},
        {NULL, NULL},
    };

    easy_menu_loop(menu, "操作员汇总");

}

static void oper_login()
{
    if (p16pos.login_flag == 1)
    {
        disp_msg("已签到,请先签出", 10);
        return;
    }
    if (!check_oper_login())
    {
        disp_msg("签到成功", 5);
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
        disp_msg("无流水", 10);
        return;
    }
    else if (ret < 0)
    {
        disp_msg("查询流水失败,请联系管理员", 10);
    }
    else
    {
        ret = pos_printer(&transdtl, 1, 1);
        if (ret == 0)
        {
            disp_msg("打印成功", 5);
        }
    }
}
static void oper_printer_by_devseqno()
{
    CLEAR_SCREEN;
    input_dst* devseqno_input;
    devseqno_input = input_init(FREE_MOD);

    input_props_set(devseqno_input, input_one_property);
    input_property_set(devseqno_input, INPUT_HIND, "流水号");
    input_property_set(devseqno_input, INPUT_MAX_LEN, 10);
    input_dis(MAIN_DISP, devseqno_input);

    printf_str(MAIN_DISP, DISP_X_COOR, DISP_Y_COOR, "输入终端流水号", 1);

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
        disp_msg("无流水", 10);
        return;
    }
    else if (ret < 0)
    {
        disp_msg("查询流水失败,请联系管理员", 10);
    }
    else
    {
        if (transdtl.transflag == TRANS_SYS_FAILED || transdtl.transflag == TRANS_INIT)
        {
            disp_msg("无效流水不能打印", 3);
            return;
        }
        ret = pos_printer(&transdtl, 1, 1);
        if (ret == 0)
        {
            disp_msg("打印成功", 5);
        }
    }
}
static void oper_pos_printer()
{
    if (p16pos.login_flag != 1)
    {
        disp_msg("请操作员先登录", 10);
        return;
    }
    menu_item_t menu[] =
    {
        {"1.最后一笔", &oper_printer_last},
        {"2.按流水号", &oper_printer_by_devseqno},
        {NULL, NULL},
    };
    easy_menu_loop(menu, "小票打印");
}

static void deposit_menu()
{
    menu_item_t menu[] =
    {
        {"1.现金充值", &deposit_main},
        {"2.操作员汇总", &statistic_main},
        {"3.操作员签到", &oper_login},
        {"4.小票补打", &oper_pos_printer},
        {"5.操作员签出", &oper_logout},
        {NULL, NULL},
    };

    easy_menu_loop(menu, "现金充值");
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

void init_purchase_menu()
{
    struct pic_lst* main_menu_ptr;
    char* main_menu_table[] =
    {
        PIC4, "消费",
        PIC1, "参数设置",
        PIC6, "关机",
        NULL, NULL
    };

    main_menu_ptr = piclist_init();
    if (NULL == main_menu_ptr)
    {
        error_exit(2, "初始化屏幕对象失败");
    }
    jpg_display(MAIN_DISP, 0, 0, DESKTOP_JPG) ;  //gepspxs19.jpg //  ltby.jpg
    jpg_stormem(MAIN_DISP, MAIN_DISP->bak_ptr, 0, 0, DESKTOP_JPG);

    piclist_props_set(main_menu_ptr, main_menu_table, main_menu_property);
    piclist_show(p16pos.main_disp, main_menu_ptr);
    // 主菜单循环
    while (1)
    {
        // CLEAR(MAIN_DISP, DISP_X_COOR - 5, DISP_Y_COOR - 5,
        //       DISP_X_COOR + DISP_X_PIXEL + 10, DISP_Y_COOR + DISP_Y_PIXEL + 10);
        CLEAR_SCREEN;
        piclist_show(MAIN_DISP, main_menu_ptr);
        lcd_160_upd();

        int32 ret = piclist_key_manege(MAIN_DISP, main_menu_ptr, lcd_160_upd);
        //LOG( ( LOG_DEBUG, "菜单按键，key=%d", ret ) );
        switch (ret)
        {
        case 0:
			menu_purchase_input();
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


void init_menu()
{
    struct pic_lst* main_menu_ptr;
    char work_menu_name [64] = {0};
    char* main_menu_table[] =
    {
        PIC4, work_menu_name,
        PIC1, "参数设置",
        PIC6, "关机",
        NULL, NULL
    };

    if (p16pos.work_mode == 0)
    {
        strcpy(work_menu_name, "现金充值");
    }
    else
    {
        strcpy(work_menu_name, "补助领取");
    }

    main_menu_ptr = piclist_init();
    if (NULL == main_menu_ptr)
    {
        error_exit(2, "初始化屏幕对象失败");
    }
    jpg_display(MAIN_DISP, 0, 0, DESKTOP_JPG) ;  //gepspxs19.jpg //  ltby.jpg
    jpg_stormem(MAIN_DISP, MAIN_DISP->bak_ptr, 0, 0, DESKTOP_JPG);

    piclist_props_set(main_menu_ptr, main_menu_table, main_menu_property);
    piclist_show(p16pos.main_disp, main_menu_ptr);
    // 主菜单循环
    while (1)
    {
        // CLEAR(MAIN_DISP, DISP_X_COOR - 5, DISP_Y_COOR - 5,
        //       DISP_X_COOR + DISP_X_PIXEL + 10, DISP_Y_COOR + DISP_Y_PIXEL + 10);
        CLEAR_SCREEN;
        piclist_show(MAIN_DISP, main_menu_ptr);
        lcd_160_upd();

        int32 ret = piclist_key_manege(MAIN_DISP, main_menu_ptr, lcd_160_upd);
        //LOG( ( LOG_DEBUG, "菜单按键，key=%d", ret ) );
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

