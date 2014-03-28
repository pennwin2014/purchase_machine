/******************************************************************************
* Copyright SupWisdom 2012 all right reserved
* Module: Main module
* File: p16main.c
* Auth: Cheng Tang<cheng.tang@supwisdom.com>
******************************************************************************/

#include <stdlib.h>
#include <limits.h>
#include <libgen.h>
#include <unistd.h>

#include "config.h"
#include "p16log.h"
#include "menu.h"
#include "syspara.h"
#include "service.h"
#include "utils.h"
#include "statusbar.h"
#include "gnudes.h"
#include "jpeglib.h"
#include "cardop.h"


p16_context p16pos;
p16_card_context p16card;

// {{{ 初始化系统参数
static int do_init_all_hardware()
{
    int32 ret;
    ret = tft_init();
    if (ret)
    {
        error_exit(1, "不能初始化LCD显示");
    }
    lcd_160_clr();

    ret = key_led_init();
    if (ret)
    {
        error_exit(1, "不能初始化keyboard");
    }
    //led_dis_clear();

    RFDEV = dc_init_rf();
    if (RFDEV < 0)
    {
        error_exit(1, "不能初始化射频卡模块");
    }
    dc_config_card(RFDEV, 'A');
    if (dc_typeb_timeout(RFDEV, 9))
    {
        error_exit(1, "dc_typeb_timeout错误");
    }

    ret = fs_print_init();
    if (ret)
    {
        p16log(LOG_ERROR, "不能初始化打印机模块");
    }
    SAMDEV = dc_int_sam();
    if (SAMDEV < 0)
    {
        p16log(LOG_ERROR, "不能初始化SAM模块，%d", SAMDEV);
    }
    return 0;
}
//}}}
// {{{ 加载配置文件
/**
 * @brief 加载配置文件
 * @return - 返回0 表示成功，其它表示失败
 */
static int do_load_config()
{
    int ret = sam_read_termno(p16pos.termno);
    if (!ret)
    {
        p16pos.load_sam = 1;
    }
    else
    {
        p16pos.load_sam = 0;
    }

    MAIN_DISP = ds_init(480, 272, TFT_16);
    if (NULL == MAIN_DISP)
    {
        error_exit(2, "初始化屏幕显示失败");
    }

    return 0;
}
// }}}
// {{{ load_ip_config
static int load_ip_config()
{
    char ip[24] = {0};
    char netmask[24] = {0};
    char gw[24] = {0};
    if (SAFE_GET_CONFIG("dev.ip", ip)
            || SAFE_GET_CONFIG("dev.netmask", netmask)
            || SAFE_GET_CONFIG("dev.gateway", gw))
    {
        LOG((LOG_ERROR, "获取IP配置失败"));
        return -1;
    }
    return set_local_ip(ip, netmask, gw);
}
// }}}
// {{{ load_sys_config
static int load_sys_config()
{
    if (SAFE_GET_CONFIG("dev.devphyid", p16pos.devphyid))
        return -1;
    //LOG((LOG_DEBUG,"devphyid: %s",p16pos.devphyid));
    if (SAFE_GET_CONFIG("sys.appid", p16pos.appid))
        return -1;
    if (SAFE_GET_CONFIG("sys.appsecret", p16pos.appsecret))
        return -1;

    if (SAFE_GET_CONFIG("svc.remotename", p16pos.remote[REMOTE_WIRE].u.host.host_name))
    {
        return -1;
    }
    if (get_config_int("svc.remoteport", &(p16pos.remote[REMOTE_WIRE].u.host.host_port)))
    {
        return -1;
    }
    if (SAFE_GET_CONFIG("svc.remoteurl", p16pos.service_path))
    {
        return -1;
    }
    int pages;
    if (get_config_int("sys.printerpages", &pages))
    {
        return -1;
    }
    int cfgverno;
    if (get_config_int("sys.cfgverno", &cfgverno))
    {
        p16pos.cfgverno = 0;
    }
    else
    {
        p16pos.cfgverno = cfgverno;
    }
    p16pos.printer_pages = (uint8)pages;

    if (get_config_int("sys.workmode", &(p16pos.work_mode)))
    {
        p16pos.work_mode = 0;
    }

    if (get_config_int("sys.heartbeatminutes", &(p16pos.heartbeat_minutes)))
    {
        p16pos.heartbeat_minutes = 0;
    }
    //memset( p16pos.termno, 0, sizeof p16pos.termno );
    //memcpy(p16pos.termno,"\x00\x00\x00\x05\x00\x04",6);
    setup_svc_url();
    return 0;
}
// }}}
// {{{ clear_transdtl
static int clear_transdtl()
{
    char check_day[15] = {0};
    int ret;
    get_datetime_offset(-(SECONDS_ONE_DAY * 7), "%Y%m%d", check_day);
    ret = trans_clear_expire_dtl(check_day);
    if (ret)
    {
        return -1;
    }
    return 0;
}
// }}}
// {{{ statusbar 相关函数
static void set_batt_icon(struct pic_lst* ptr , uint8 n , uint8 n_icon)
{
    if (1 == n_icon)
    {
        piclist_modify(ptr, n, DIANCI1_BMP, NULL) ;
    }
    else if (2 == n_icon)
    {
        piclist_modify(ptr, n, DIANCI2_BMP, NULL) ;
    }
    else if (3 == n_icon)
    {
        piclist_modify(ptr, n, DIANCI3_BMP, NULL) ;
    }
    else if (4 == n_icon)
    {
        piclist_modify(ptr, n, DIANCI4_BMP, NULL) ;
    }
    else if (0 == n_icon)
    {
        piclist_modify(ptr, n, DIANCI0_BMP, NULL) ;
    }
    else
    {

    }
}

void batt_state_get(struct pic_lst* ptr)
{
    uint8       batt_vol ;
    // static       int tim = 0 ;
    batt_vol = battey_vol_get() ;
    set_batt_icon(ptr , 1 , batt_vol);

    // if(tim & 0x01)
    // {
    //   if((0 == batt_vol) || (4 == batt_vol))
    //   {
    //     piclist_property_set(ptr, BMP_DMOD_SET, 1 , 1) ;
    //   }
    // }
    // else
    // {
    //   if((0 == batt_vol) || (4 == batt_vol))
    //   {
    //     piclist_property_set(ptr, BMP_DMOD_SET, 1 , 0) ;
    //   }
    //   else
    //   {
    //     piclist_property_set(ptr, BMP_DMOD_SET, 1 , 1) ;
    //   }
    // }
    // tim++;
    piclist_property_set(ptr, BMP_DMOD_SET, 1 , 1) ;
}

void power_sig_get(struct pic_lst* ptr)
{
    int     ret ;
    ret = power_sta_get() ;
    if (ret != 0)
    {
        piclist_modify(ptr, 2, DIANYUAN_FS, NULL) ;
    }
    else
    {
        piclist_modify(ptr, 2, DIANYUAN_OK, NULL) ;
    }
}

static void show_statusbar_draw(dis_map* disp, void* args)
{
    /*
    static int8 *sb_icon_tab[] =
    {
        XINHAO0_BMP,    NULL,
        DIANCI0_BMP,    NULL,
        LIANJIE_BMP,    NULL,
        DIANYUAN_OK,    NULL,
        NULL,           NULL
    };
    */
    static char*    sb_icon_tab[] =
    {
        LIANJIE_BMP,    NULL,
        DIANCI0_BMP,    NULL,
        DIANYUAN_OK,  NULL,
        NULL,           NULL
    };
    static proptey  time_lbl_props[] =
    {
        {LABL_STA_STATION , 120 , 6},
        {LABL_STATE ,       STA_RIGHT , 0},
        {LABL_WIDTH ,       80 , 10},
        {LABL_FONT_TYPE ,   HZ_16DZ , ASC_16DZ},
        // {LABL_BAK_COLOR, WHITE_COLR, 0},
        {0xffff ,           0 , 0}
    };
    static proptey oper_status_lbl_props[] =
    {
        {LABL_STA_STATION , 300 , 6},
        {LABL_STATE ,     STA_LEFT , 0},
        {LABL_WIDTH ,     120 , 10},
        {LABL_FONT_TYPE ,   HZ_16DZ , ASC_16DZ},
        // {LABL_BAK_COLOR, WHITE_COLR, 0},
        {0xffff ,       0 , 0}
    };
    uint32 seconds = 0;
    struct pic_lst* statusbar_ptr;
    statusbar_ptr = piclist_init();
    if (NULL == statusbar_ptr)
    {
        return;
    }
    piclist_props_set(statusbar_ptr, sb_icon_tab, NULL);
    piclist_property_set(statusbar_ptr, BMP_STAT_SET , 2, 1);
    piclist_show(disp, statusbar_ptr);

    label_dst* time_lbl;
    time_lbl = label_init();
    if (NULL == time_lbl)
    {
        return;
    }
    label_props_set(time_lbl, time_lbl_props);
    char datetime[64] = {0};
    SAFE_GET_DATETIME("%Y/%m/%d %H:%M:%S", datetime);
    label_property_set(time_lbl, LABL_DIS_CAPTION, datetime);
    label_dis_bak(disp, time_lbl);
    // label_dis(disp, time_lbl);

    label_dst* oper_lbl;
    char oper_status[31];
    oper_lbl = label_init();
    if (NULL == oper_lbl)
    {
        return;
    }
    label_props_set(oper_lbl, oper_status_lbl_props);
    uint8 login_flag;
    if (!p16pos.login_flag)
    {
        strcpy(oper_status, "未登录");
    }
    else
    {
        if (p16pos.work_mode == 0)
        {
            snprintf(oper_status, 30, "操作员: %s", p16pos.opername);
        }
        else
        {
            snprintf(oper_status, 30, "终端已登录");
        }
    }
    login_flag = p16pos.login_flag;
    label_property_set(oper_lbl, LABL_DIS_CAPTION, oper_status);
    label_dis_bak(disp, oper_lbl);
    CLEAR(disp, 1, 1, SCREEN_X_PIXEL, 26);
    while (p16pos.app_running)
    {

        if (seconds % 10 == 0 || login_flag != p16pos.login_flag)
        {
            CLEAR(disp, 1, 1, SCREEN_X_PIXEL, 26);
            if (net_sta_get() == 1)
            {
                piclist_modify(statusbar_ptr, 0, LIANJIE_OK, NULL) ;
            }
            else
            {
                piclist_modify(statusbar_ptr, 0, LIANJIE_FS, NULL) ;
            }
            batt_state_get(statusbar_ptr);
            power_sig_get(statusbar_ptr);

            if (!p16pos.login_flag)
            {
                strcpy(oper_status, "未登录");
            }
            else
            {
                snprintf(oper_status, 30, "操作员: %s", p16pos.opername);
            }
            label_property_set(oper_lbl, LABL_DIS_CAPTION, oper_status);
            label_dis_bak(disp, oper_lbl);
            piclist_show(disp, statusbar_ptr);
            login_flag = p16pos.login_flag;
        }
        else
        {
            CLEAR(disp, 120, 1, 300, 26);
        }
        memset(datetime, 0, sizeof datetime);
        SAFE_GET_DATETIME("%Y/%m/%d %H:%M:%S", datetime);
        label_property_set(time_lbl, LABL_DIS_CAPTION, datetime);
        // label_dis(disp, time_lbl);
        label_dis_bak(disp, time_lbl);
        lcd_160_upd() ;
        sleep(1);
        ++seconds;
    }
    LOG((LOG_DEBUG, "show_statusbar_draw end"));
    label_destory(time_lbl);
}
static void sys_statusbar(void* args)
{
    dis_map* disp = ds_init(SCREEN_X_PIXEL, 272, TFT_16);
    if (NULL == disp)
    {
        LOG((LOG_ERROR, "创建状态栏显示对象失败"));
        return;
    }
    jpg_display(disp, 0, 0, "../res/jpg/bak.jpg") ;
    jpg_stormem(disp, disp->bak_ptr, 0, 0, "../res/jpg/bak.jpg");
    show_statusbar_draw(disp, args);
    ds_destory(disp);
    pthread_exit(0);
}

static void show_statusbar()
{
    pthread_t pid;
    int ret;
    ret = pthread_create(&pid, NULL, (void*)&sys_statusbar, NULL);
    if (ret)
    {
        LOG((LOG_ERROR, "创建显示状态栏线程失败"));
        return;
    }
}
//}}}
// {{{ test_printer
static void test_printer()
{
    PRINTER = ds_init(FS_DOT_NUM, 640 , BIT_L);
    if (NULL == PRINTER)
        return;
    /*
    dis_mod_set(PRINTER ,HZ_32DZ ,ASC_32DZ);
    printf_str(PRINTER, 120, 1, "一卡通充值机", 1);
    char tip[64];
    sprintf(tip,"终端号: %s",p16pos.devphyid);
    printf_str(PRINTER, 30, 50, tip, 1);
    char date[24] = {0};
    SAFE_GET_DATETIME("%Y/%m/%d",date);
    sprintf(tip,"日期: %s",date);
    printf_str(PRINTER, 30, 90, tip, 1);

    memset(date,0,sizeof date);
    SAFE_GET_DATETIME("%H:%M:%S",date);
    sprintf(tip,"时间: %s",date);
    printf_str(PRINTER, 30, 130, tip, 1);

    fs_print_dat(PRINTER->ptr,PRINTER->buf_size) ;
    int ret;
    uint8 step;
    while(1)
    {
        ret = fs_print_status(&step);
        if(0 == ret)
        {
            disp_msg("打印测试完成",10);
            return;
        }
        else if(-1==ret)
        {
            disp_msg("打印失败",10);
        }
        else
        {
            continue;
        }
    }
     */
}

#if 0
static void test_des()
{
    uint8 key[] = "\x24\xC9\xA5\x96\x27\x8B\xFD\x79";
    uint8 data[] = "\xDD\xE2\xC4\xDB\x22\x1D\x3B\x24";
    uint8 result[8];
    des_context ctx;
    LOG((LOG_DEBUG, "uint32=%d, uint16=%d", sizeof(uint32), sizeof(uint16)));
    memset(&ctx, 0, sizeof ctx);
    des_set_key(&ctx, key);
    des_encrypt(&ctx, data, result);
    p16dump_hex(LOG_ERROR, result, 8);
}
#endif
//}}}
//{{{ do_check_desktop
#if 1
static void do_check_desktop()
{
    FILE* fp;
    int offset;
    if (is_file_exists(DESKTOP_JPG))
        return;
    fp = fopen(DESKTOP_JPG, "wb");
    if (!fp)
        return;
    struct jpeg_compress_struct jpeg;
    struct jpeg_error_mgr  jerr;
    jpeg.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&jpeg);

    jpeg_stdio_dest(&jpeg, fp);

    jpeg.image_width = SCREEN_X_PIXEL;
    jpeg.image_height = SCREEN_Y_PIXEL;
    jpeg.input_components = 3;
    jpeg.in_color_space   = JCS_RGB;

    jpeg_set_defaults(&jpeg);
    jpeg_set_quality(&jpeg, 80, 1);

    JSAMPROW raw_data[1];
    int row_stride;

    row_stride = jpeg.image_width;
    unsigned char data[4096];
    unsigned char rgb[] = "\xba\xd1\xe8";
    for (offset = 0; offset < sizeof(data); ++offset)
    {
        data[offset] = rgb[offset % 3];
    }
    //memset(data, 0x, sizeof data);
    jpeg_start_compress(&jpeg, 1);
    while (jpeg.next_scanline < jpeg.image_height)
    {
        raw_data[0] = &data[0];
        jpeg_write_scanlines(&jpeg, raw_data, 1);
    }

    jpeg_finish_compress(&jpeg);
    fclose(fp);
    jpeg_destroy_compress(&jpeg);
}
#else
static void do_check_desktop()
{
    const char temp[] = "../res/jpg/gepspxs19.jpg";
    if (is_file_exists(DESKTOP_JPG))
        return;
    uint8 buffer[4096];
    FILE* src, *dst;
    src = fopen(temp, "rb");
    dst = fopen(DESKTOP_JPG, "w+b");
    if (src == NULL || dst == NULL)
    {
        return;
    }
    int readlen;
    while (1)
    {
        readlen = fread(buffer, 1, sizeof(buffer), src);
        if (readlen <= 0)
            break;
        fwrite(buffer, readlen, 1, dst);
    }
    fclose(src);
    fclose(dst);
}
#endif
//}}}


int main(int argc, char* const argv[])
{
    memset(&p16pos, 0, sizeof p16pos);
	memset(&p16card, 0, sizeof p16card);

    char temp[PATH_MAX + 1];
    realpath(argv[0], temp);
    strcpy(p16pos.base_dir, dirname(temp));
    // LOG((LOG_DEBUG, "启动路径: %s", p16pos.base_dir));

    int8 ret;
    ret = check_config_db();
    if (ret)
    {
        error_exit(1, "初始化参数失败");
    }
    ret = load_sys_config();
    if (ret)
    {
        error_exit(2, "加载参数失败");
    }

    load_ip_config();
    // 初始化参数
    do_init_all_hardware();
    // 加载配置信息
    do_load_config();

    clear_transdtl();

    test_printer();
    do_check_desktop();

    p16pos.app_running = 1;
    show_statusbar();

    // 显示主菜单
	menu_purchase_input();

    return 0;
}
// vim:foldmethod=marker

