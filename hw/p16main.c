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

// {{{ ��ʼ��ϵͳ����
static int do_init_all_hardware()
{
    int32 ret;
    ret = tft_init();
    if (ret)
    {
        error_exit(1, "���ܳ�ʼ��LCD��ʾ");
    }
    lcd_160_clr();

    ret = key_led_init();
    if (ret)
    {
        error_exit(1, "���ܳ�ʼ��keyboard");
    }
    //led_dis_clear();

    RFDEV = dc_init_rf();
    if (RFDEV < 0)
    {
        error_exit(1, "���ܳ�ʼ����Ƶ��ģ��");
    }
    dc_config_card(RFDEV, 'A');
    if (dc_typeb_timeout(RFDEV, 9))
    {
        error_exit(1, "dc_typeb_timeout����");
    }

    ret = fs_print_init();
    if (ret)
    {
        p16log(LOG_ERROR, "���ܳ�ʼ����ӡ��ģ��");
    }
    SAMDEV = dc_int_sam();
    if (SAMDEV < 0)
    {
        p16log(LOG_ERROR, "���ܳ�ʼ��SAMģ�飬%d", SAMDEV);
    }
    return 0;
}
//}}}
// {{{ ���������ļ�
/**
 * @brief ���������ļ�
 * @return - ����0 ��ʾ�ɹ���������ʾʧ��
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
        error_exit(2, "��ʼ����Ļ��ʾʧ��");
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
        LOG((LOG_ERROR, "��ȡIP����ʧ��"));
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
	//��ȡ�������汾��
	char pos_blacklist_version[13] = {0};
	if(SAFE_GET_CONFIG("dev.pos_blacklist_version",pos_blacklist_version))
		return -1;
	decode_hex(pos_blacklist_version, 12,p16pos.pos_blacklist_version);
    //LOG((LOG_DEBUG,"devphyid: %s",p16pos.devphyid));
    if (SAFE_GET_CONFIG("sys.appid", p16pos.appid))
        return -1;
    if (SAFE_GET_CONFIG("sys.appsecret", p16pos.appsecret))
        return -1;
	//��ȡǰ�õ�ip�Ͷ˿ں�
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
		LOG((LOG_DEBUG,"��ȡsys.cfgvernoʧ��"));
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
// {{{ statusbar ��غ���
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
        strcpy(oper_status, "δ��¼");
    }
    else
    {
        if (p16pos.work_mode == 0)
        {
            snprintf(oper_status, 30, "����Ա: %s", p16pos.opername);
        }
        else
        {
            snprintf(oper_status, 30, "�ն��ѵ�¼");
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
                strcpy(oper_status, "δ��¼");
            }
            else
            {
                snprintf(oper_status, 30, "����Ա: %s", p16pos.opername);
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
        LOG((LOG_ERROR, "����״̬����ʾ����ʧ��"));
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
        LOG((LOG_ERROR, "������ʾ״̬���߳�ʧ��"));
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
    printf_str(PRINTER, 120, 1, "һ��ͨ��ֵ��", 1);
    char tip[64];
    sprintf(tip,"�ն˺�: %s",p16pos.devphyid);
    printf_str(PRINTER, 30, 50, tip, 1);
    char date[24] = {0};
    SAFE_GET_DATETIME("%Y/%m/%d",date);
    sprintf(tip,"����: %s",date);
    printf_str(PRINTER, 30, 90, tip, 1);

    memset(date,0,sizeof date);
    SAFE_GET_DATETIME("%H:%M:%S",date);
    sprintf(tip,"ʱ��: %s",date);
    printf_str(PRINTER, 30, 130, tip, 1);

    fs_print_dat(PRINTER->ptr,PRINTER->buf_size) ;
    int ret;
    uint8 step;
    while(1)
    {
        ret = fs_print_status(&step);
        if(0 == ret)
        {
            disp_msg("��ӡ�������",10);
            return;
        }
        else if(-1==ret)
        {
            disp_msg("��ӡʧ��",10);
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
/////////////////////////////�������////////////////////////////////////////
static void get_blacklist_data(p16_tcp_blacklist_data* ptcp_blacklist_data)
{
	//get_order_array_by_int(0,ptcp_blacklist_data->machine_no,4);
	//memset(ptcp_blacklist_data->pos_blacklist_version,0,6);

	decode_hex(p16pos.devphyid, 8, ptcp_blacklist_data->machine_no);
	memcpy(ptcp_blacklist_data->pos_blacklist_version,p16pos.pos_blacklist_version, 6);
}


static void get_heart_data(p16_tcp_heart_data* ptcp_heart_data)
{
	ptcp_heart_data->identify_code = 0x01;		
	get_order_array_by_int(p16pos.cfgverno, ptcp_heart_data->cfgverno, 4);
	decode_hex(p16pos.devphyid, 8, ptcp_heart_data->machine_no);
	//get_order_array_by_int(5,ptcp_heart_data->machine_no,4);//����devphyid
}


static int get_rt_transdtl_data(p16_tcp_transdtl* ptcp_rt_transdtl)
{
	int ret = 0;
	p16_transdtl_t transdtl;
	memset(&transdtl,0,sizeof transdtl);
	ret = trans_get_last_record(&transdtl);
	if(ret == 0)
	{
		//LOG((LOG_DEBUG,"�޿�ȡ��������ˮ"));
		return -1;
	}
	//��������¼��Ϊ�Ѷ�ȡ
	transdtl.confirm = 1;
	ret = trans_update_record(&transdtl);
	if(ret)
	{
		//LOG((LOG_ERROR,"���½�����ˮʧ��"));
		return -2;
	}
	ptcp_rt_transdtl->devseqno= transdtl.devseqno;//�����devseqno
	get_order_array_by_int(transdtl.cardbefbal, ptcp_rt_transdtl->cardbefbal, 4);
	get_order_array_by_int(transdtl.paycnt, ptcp_rt_transdtl->paycnt, 2);
	get_order_array_by_int(transdtl.cardno, ptcp_rt_transdtl->cardno, 4);
	if(transdtl.transflag == TRANS_WRITE_CARD_OK)
	{
		ptcp_rt_transdtl->transflag = 0x81;//10000001
	}
	else if(transdtl.transflag == TRANS_WRITE_CARD_FAILED)
	{
		ptcp_rt_transdtl->transflag = 0x83;//10000011
	}
	get_order_array_by_int(0, ptcp_rt_transdtl->additional, 3);//���ӽ����ʱΪ0
	get_order_array_by_int(transdtl.amount, ptcp_rt_transdtl->transamt, 4);
	decode_hex(transdtl.termno, 12, ptcp_rt_transdtl->termno);
	get_order_array_by_int(0, ptcp_rt_transdtl->machine_no, 4);//��������ʱ��0
	char transdatetime[15] = {0};
	SAFE_GET_DATETIME("%y%m%d%H%M%S", transdatetime);
	get_std_trans_datetime(transdatetime, ptcp_rt_transdtl->transdatetime, 6);
	//decode_hex(transdatetime, 12, ptcp_rt_transdtl->transdatetime);//����ʱ��
	
		//��ӡ����ʱ��
	decode_hex(transdtl.tac, 8, ptcp_rt_transdtl->tac);
	int buffer_len = sizeof(p16_tcp_transdtl)-2;
	LOG((LOG_DEBUG,"����crc��buffer_len=%d",buffer_len));
	pb_protocol_crc((const uint8 *)ptcp_rt_transdtl, buffer_len, ptcp_rt_transdtl->crc);
	return 0;
}



static int get_batch_transdtl_data(p16_tcp_transdtl* ptcp_transdtl, uint8* pdata_out_cnt)
{
	int init_len = (*pdata_out_cnt);
	LOG((LOG_DEBUG,"����get_batch_transdtl_data,data_out_cnt=%d",init_len));
	int i = 0;
	int ret = 0;
	p16_transdtl_t transdtl;
	for(i=0; i<init_len; i++)
	{
		LOG((LOG_DEBUG,"��%d��",i));
		memset(&transdtl,0,sizeof transdtl);
		ret = trans_get_last_record(&transdtl);
		if(ret == 0)
		{
			LOG((LOG_DEBUG,"ȡ������ˮ��������%d��",i));
			break;
		}
		else if(ret == -1)
		{
			LOG((LOG_DEBUG,"ȡ������ˮʧ�ܣ���%d��",i));
			break;
		}
		//��������¼��Ϊ�Ѷ�ȡ
		transdtl.confirm = 1;
		ret = trans_update_record(&transdtl);
		if(ret)
		{
			LOG((LOG_ERROR,"���½�����ˮʧ�ܣ���%d��",i));
			break;
		}
		ptcp_transdtl[i].devseqno= transdtl.devseqno;//�����devseqno
		//memcpy(ptcp_transdtl[i].cardbefbal, &transdtl.cardbefbal, 4);
		get_order_array_by_int(transdtl.cardbefbal, ptcp_transdtl[i].cardbefbal, 4);
		LOG((LOG_DEBUG,"������ǰ���=%d",transdtl.cardbefbal));
		p16dump_hex(LOG_DEBUG, ptcp_transdtl[i].cardbefbal, 4);
		get_order_array_by_int(transdtl.paycnt, ptcp_transdtl[i].paycnt, 2);
		get_order_array_by_int(transdtl.cardno, ptcp_transdtl[i].cardno, 4);
		//ptcp_transdtl[i].transflag = 0x00;//���ѳɹ����ڿ����ѣ�д���ɹ���д��ʧ��
		if(transdtl.transflag == TRANS_WRITE_CARD_OK)
		{
			ptcp_transdtl[i].transflag = 0x81;//10000001
		}
		else if(transdtl.transflag == TRANS_WRITE_CARD_FAILED)
		{
			ptcp_transdtl[i].transflag = 0x83;//10000011
		}
		get_order_array_by_int(0, ptcp_transdtl[i].additional, 3);//���ӽ����ʱΪ0
		get_order_array_by_int(transdtl.amount, ptcp_transdtl[i].transamt, 4);
		decode_hex(transdtl.termno, 12, ptcp_transdtl[i].termno);
		get_order_array_by_int(0, ptcp_transdtl[i].machine_no, 4);//��������ʱ��0
		char transdatetime[15] = {0};
		SAFE_GET_DATETIME("%y%m%d%H%M%S", transdatetime);
		get_std_trans_datetime(transdatetime, ptcp_transdtl[i].transdatetime, 6);
		//decode_hex(transdatetime, 12, ptcp_transdtl[i].transdatetime);//����ʱ��
		//��ӡ����ʱ��
		decode_hex(transdtl.tac, 8, ptcp_transdtl[i].tac);
		int buffer_len = sizeof(ptcp_transdtl[i])-2;
		LOG((LOG_DEBUG,"buffer_len=%d",buffer_len));
		pb_protocol_crc((const uint8 *)&ptcp_transdtl[i], buffer_len, ptcp_transdtl[i].crc);
	}
	if(i == 0)
	{
		LOG((LOG_DEBUG,"��������ˮ��Ҫ�ϴ�"));
		return -1;
	}
	(*pdata_out_cnt) = i; 		
	return 0;
}
//////////////////////////////���ͺ���////////////////////////////////////
static void send_download_blacklist_data(char* recv_buf, int* precv_len)
{
	p16_tcp_blacklist_package blacklist_package;
	memcpy(blacklist_package.guide_code,"HST",3);
	blacklist_package.cmd_code = 0xDA;//���غ�����
	get_order_array_by_int(0, blacklist_package.machine_addr, 2);
	blacklist_package.package_len = 2+10;//������ַ+���ݰ�����
	get_blacklist_data(&blacklist_package.data);
	int send_len = 0;
	char send_buff[2048] = {0};
	pack_tcp_blacklist_data(&blacklist_package, send_buff, &send_len);
	if(send_len>0)
	{
		//��ӡ��������
		LOG((LOG_DEBUG,"���������͵Ĵ���ӡ:"));
		p16dump_hex(LOG_DEBUG,send_buff,send_len);
		int ret = 0;
		ret = tcp_send_data(send_buff, send_len, recv_buf, precv_len);
		if(ret)
		{
			//tcp���ʹ���
			return;
		}
	}
}

static void send_heart_data(char* recv_buf, int* precv_len)
{
	p16_tcp_heart_package heart_package;
	memcpy(heart_package.guide_code,"HST",3);
	heart_package.cmd_code = 0xD5;//����
	get_order_array_by_int(0,heart_package.machine_addr,2);//������ַ��ʱ��Ϊ0
	//���ݰ�����Ϊ������ַ��������Ϣ������
	heart_package.package_len =  9 + 2;
	get_heart_data(&heart_package.data);
	int send_len = 0;
	char send_buff[2048] = {0};
	pack_tcp_heart_data(&heart_package, send_buff, &send_len);
	if(send_len>0)
	{
		//��ӡ��������
		LOG((LOG_DEBUG,"�������͵Ĵ���ӡ:"));
		p16dump_hex(LOG_DEBUG,send_buff,send_len);
		int ret = 0;
		ret = tcp_send_data(send_buff, send_len, recv_buf, precv_len);
	}

}

static int send_rt_transdtl_data(char* recv_buf, int* precv_len)
{
	p16_tcp_rt_package package;
	memset(&package,0,sizeof package);
	memcpy(package.guide_code,"HST",3);
	package.cmd_code = 0xD2;//ʵʱ������ˮ
	get_order_array_by_int(0,package.machine_addr,2);//������ַ��ʱ��Ϊ0
	int ret = 0;
	ret = get_rt_transdtl_data(&package.data);
	if(ret)
	{
		LOG((LOG_DEBUG,"���Է���ʵʱ��ˮʧ�ܣ�ԭ��=%d",ret));
		return -1;
	}
	//���ݰ�����Ϊ������ַ��������Ϣ������
	package.package_len =  sizeof(package.data) + sizeof(package.machine_addr);
	
	int send_len = 0;
	char send_buff[2048] = {0};
	pack_tcp_rt_transdtl_data(&package, send_buff, &send_len);
	if(send_len>0)
	{
		//��ӡ��������
		LOG((LOG_DEBUG,"ʵʱ��ˮ���͵Ĵ���ӡ:"));
		p16dump_hex(LOG_DEBUG,send_buff,send_len);
		int ret = 0;
		ret = tcp_send_data(send_buff, send_len, recv_buf, precv_len);
	}
	return 0;

}

static int send_batch_transdtl_data(char* recv_buf, int* precv_len)
{
	int ret = 0;
	char send_buff[2048];//���շ��͵�����
	p16_tcp_batch_package batch_package;
	uint32 send_len;
	memset(&batch_package,0,sizeof batch_package);
	batch_package.data.tcp_transdtl_cnt = MAX_TCP_TRANSDTL_CNT;
	ret = get_batch_transdtl_data(batch_package.data.tcp_transdtl_list, &batch_package.data.tcp_transdtl_cnt);
	if(ret)
	{
		LOG((LOG_DEBUG,"�����ϴ�getdataʧ��"));
		return -1;
	}
	if(batch_package.data.tcp_transdtl_cnt>0)
	{
		memset(send_buff,0,2048);
		memcpy(batch_package.guide_code,"HST",3);
		batch_package.cmd_code = 0xDB;//�����ϴ�������ˮ
		get_order_array_by_int(0,batch_package.machine_addr,2);
		//���ݰ�����Ϊ������ַ��������Ϣ������
		batch_package.package_len = sizeof(batch_package.machine_addr)+ 
					sizeof(batch_package.data.tcp_transdtl_cnt) + 
					sizeof(p16_tcp_transdtl)*batch_package.data.tcp_transdtl_cnt;
		LOG((LOG_DEBUG,"������ˮpackage_len=%d",batch_package.package_len));
		send_len = 0;
		pack_tcp_batch_data(&batch_package, send_buff, &send_len);//���ڲ���ÿ�ζ������������
		if(send_len>0)
		{
			//��ӡ��������
			LOG((LOG_DEBUG,"send_len=%d,send_buff:",send_len));
			p16dump_hex(LOG_DEBUG,send_buff,send_len);
			ret = tcp_send_data(send_buff, send_len, recv_buf, precv_len);
			/*
			//����recv_buf
			p16_recv_batch_transdtl_package recv_batch_package;
			parse_recv_data(recv_buf,1,&recv_batch_package);
			if(recv_batch_package.ret_code > 0)//??
			{
				LOG((LOG_ERROR,"�ϴ�ʧ��"));
			}
			*/
		}
	}
	return 0;

}


////////////////////////������������//////////////////////////////////////
static void parse_recv_heart(char* recv_buf, p16_recv_heart_package* recv_package)
{
	uint8 offset = 0;
	//	   53 4C 56 
	memcpy(recv_package->guide_code, recv_buf+offset, 3);
	offset += 3;
	//D5 
	recv_package->cmd_code = recv_buf[offset];
	offset += 1;
	//13 00
	memcpy(&recv_package->package_len, recv_buf+offset, 2);
	offset += 2;
	//00 00 
	memcpy(recv_package->machine_addr, recv_buf+offset, 2);
	offset += 2;
//data����
	//00 
	recv_package->data.ret_code = recv_buf[offset];
	offset += 1;
	//14 04 01 00 00 00 
	memcpy(recv_package->data.current_blacklist_version, recv_buf+offset, 6);
	offset += 6;
	//0E 04 03 0B 19 19 ==> 14 04 03 11 25 25
	memcpy(recv_package->data.server_datetime, recv_buf+offset, 6);
	offset += 6;
	//3B 00 00 00
	memcpy(recv_package->data.cfgverno, recv_buf+offset, 4);
//check
	//   7C 
	recv_package->check = recv_buf[offset];
	offset += 1;
}

static void parse_recv_rt_transdtl(char* recv_buf, p16_recv_rt_transdtl_package* recv_package)
{
	uint8 offset = 0;
	memcpy(recv_package->guide_code, recv_buf+offset, 3);
	offset += 3;
	recv_package->cmd_code = recv_buf[offset];
	offset += 1;
	memcpy(&recv_package->package_len, recv_buf+offset, 2);
	offset += 2;
	memcpy(recv_package->machine_addr, recv_buf+offset, 2);
	offset += 2;
	recv_package->data.ret_code = recv_buf[offset];
	offset += 1;
	recv_package->check = recv_buf[offset];
	offset += 1;
}

static void parse_recv_batch_transdtl(char* recv_buf, p16_recv_batch_transdtl_package* recv_package)
{
	uint8 offset = 0;
	memcpy(recv_package->guide_code, recv_buf+offset, 3);
	offset += 3;
	recv_package->cmd_code = recv_buf[offset];
	offset += 1;
	memcpy(&recv_package->package_len, recv_buf+offset, 2);
	offset += 2;
	memcpy(recv_package->machine_addr, recv_buf+offset, 2);
	offset += 2;
	recv_package->data.ret_code = recv_buf[offset];
	offset += 1;
	recv_package->check = recv_buf[offset];
	offset += 1;

}
static void parse_recv_blacklist(char* recv_buf, p16_recv_blacklist_package* recv_package)
{
	uint8 offset = 0;
	memcpy(recv_package->guide_code, recv_buf+offset, 3);
	offset += 3;
	recv_package->cmd_code = recv_buf[offset];
	offset += 1;
	memcpy(&recv_package->package_len, recv_buf+offset, 2);
	offset += 2;
	memcpy(recv_package->machine_addr, recv_buf+offset, 2);
	offset += 2;
	recv_package->data.list_cnt = recv_buf[offset];
	offset += 1;
	int i = 0;
	int tmp_int = 0, tmp_flag = 0;
	for(i=0; i<recv_package->data.list_cnt; i++)
	{
		memcpy(recv_package->data.blacklist_list[i].refno, recv_buf+offset, 4);
		offset += 4;
		recv_package->data.blacklist_list[i].flag = recv_buf[offset];
		offset += 1;
		memcpy(&tmp_int, recv_package->data.blacklist_list[i].refno, 4);
		tmp_flag = recv_package->data.blacklist_list[i].flag;
		LOG((LOG_DEBUG,"�յ�cardno=%d,flag=%d",tmp_int,tmp_flag));
	}
	memcpy(recv_package->data.sys_list_version, recv_buf+offset, 6);
	offset += 6;
	recv_package->check = recv_buf[offset];
	offset += 1;
}

static void parse_recv_data(char* recv_buf, uint8 parse_type, void* parse_struct)
{
	switch(parse_type)
	{
		case 0:	//��������	
			LOG((LOG_ERROR,"����������������"));
			parse_recv_heart(recv_buf,(p16_recv_heart_package*)parse_struct);
			break;
		case 1://�����ϴ���ˮ����
			LOG((LOG_ERROR,"���������ϴ���ˮ��������"));
			parse_recv_batch_transdtl(recv_buf,(p16_recv_batch_transdtl_package*)parse_struct);
			break;
		case 2://���غ���������
			LOG((LOG_ERROR,"�������غ�������������"));
			parse_recv_blacklist(recv_buf,(p16_recv_blacklist_package*)parse_struct);
			break;
		case 3://ʵʱ�ϴ���ˮ����
			LOG((LOG_ERROR,"����ʵʱ�ϴ���ˮ��������"));
			parse_recv_rt_transdtl(recv_buf,(p16_recv_rt_transdtl_package*)parse_struct);
			break;
		default:
			LOG((LOG_ERROR,"�յ�δ֪��������"));
			break;
	}
		
}
static int save_blackcard_record(int list_cnt, p16_recv_blacklist_unit* black_card_list)
{
	if(list_cnt <= 0)
		return -2;
	int i = 0;
	int ret = 0;
	p16_blackcard_t blackcard;
	int tmp_cardno = 0;
	for(i=0; i<list_cnt; i++)
	{
		memset(&blackcard, 0, sizeof blackcard);
		memcpy(&tmp_cardno,black_card_list[i].refno,sizeof(blackcard.cardno));
		ret = blackcard_get_record_by_cardno(&blackcard, tmp_cardno);
		blackcard.cardno = tmp_cardno;
		blackcard.cardflag = black_card_list[i].flag;
		if(ret == 0)//�ÿ��Ų����ڣ���Ҫ����һ����¼
		{
			ret = blackcard_add_record(&blackcard);
		}
		else if(ret == 1)//�ÿ��Ѿ����ڣ����¿�״̬����
		{
			ret = blackcard_update_record(&blackcard);
		}
		//
		if(ret)
		{
			//��������
			return -2;
		}
		
	}
	return 0;
}


static int check_send_transdtl()
{
	const int TRANSDTL_SLEEP_TM = 10;
	const int HEART_SLEEP_TM = 30;
	char recv_buf[1024] = {0};
	int recv_len = 0;
	int ret =0;
	LOG((LOG_DEBUG,"����check_send_transdtl"));
	while (p16pos.app_running)
	{
		memset(recv_buf,0,1024);
		recv_len = 0;
		ret = send_rt_transdtl_data(recv_buf,&recv_len);
		if(ret == 0)//����ʵʱ��ˮ�ɹ������
		{
			if(recv_len>0)
			{
				p16_recv_rt_transdtl_package recv_package;
				memset(&recv_package,0,sizeof(recv_package));
				parse_recv_data(recv_buf,3,&recv_package);
				if(recv_package.data.ret_code == 0)
				{
					LOG((LOG_DEBUG,"�ϴ�ʵʱ��ˮ�ɹ�"));
				}
				else
				{
					LOG((LOG_DEBUG,"�ϴ�ʵʱ��ˮʧ�ܣ�pos�������ϴ���ˮ"));
				}
			}
			LOG((LOG_DEBUG,"=========================sleep %d========================",TRANSDTL_SLEEP_TM));
			sleep(TRANSDTL_SLEEP_TM);
		}
		else//û�н�����ˮ��Ҫ����
		{
			//��������
			send_heart_data(recv_buf,&recv_len);
			if(recv_len>0)
			{
				//�������յ�����
				p16_recv_heart_package recv_heart;
				parse_recv_data(recv_buf,0,&recv_heart);

				LOG((LOG_DEBUG,"�����յ���ϵͳ�汾��:"));
				p16dump_hex(LOG_DEBUG,recv_heart.data.cfgverno,4);
				LOG((LOG_DEBUG,"�����յ��ķ�����ʱ��:"));
				p16dump_hex(LOG_DEBUG,recv_heart.data.server_datetime,6);
				LOG((LOG_DEBUG,"�����յ��ķ������������汾��:"));
				p16dump_hex(LOG_DEBUG,recv_heart.data.current_blacklist_version,6);
				LOG((LOG_DEBUG,"��ǰϵͳ�ĺ������汾��:"));
				p16dump_hex(LOG_DEBUG,p16pos.pos_blacklist_version, 6);
				if(memcmp(recv_heart.data.current_blacklist_version,p16pos.pos_blacklist_version, 6)>0)
				{
					LOG((LOG_DEBUG,"�����յ��ĺ������汾�Ŵ��ڵ�ǰϵͳ�ĺ������汾�ţ�ִ�����غ���������!!"));
					//���غ�����
					memset(recv_buf,0,1024);
					recv_len = 0;
					send_download_blacklist_data(recv_buf,&recv_len);
					if(recv_len>0)
					{
						p16_recv_blacklist_package recv_package;
						parse_recv_data(recv_buf, 2, &recv_package);
						if(recv_package.data.list_cnt>0)
						{
							//�к�����
							LOG((LOG_DEBUG,"�к�����%d��",recv_package.data.list_cnt));
							ret = save_blackcard_record(recv_package.data.list_cnt, recv_package.data.blacklist_list);
							if(ret)
							{
								//����������ϵ����Ա
								disp_msg("�������ݿ�ʧ�ܣ�����ϵ����Ա",10);
								return -1;
							}
							//����ϵͳ�������汾��
							char pos_blacklist_version[13] = {0};
							encode_hex(recv_package.data.sys_list_version, 6, pos_blacklist_version);
							//����ϵͳ�汾��
							ret = update_config("dev.pos_blacklist_version",pos_blacklist_version);
							if(ret)
							{
								//��������
								return -2;
							}
							memcpy(p16pos.pos_blacklist_version, recv_package.data.sys_list_version, 6);
							LOG((LOG_DEBUG,"���°汾��Ϊ:%s",pos_blacklist_version));
							LOG((LOG_DEBUG,"======================================================="));
						}
					}

				}
			}
			LOG((LOG_DEBUG,"=========================sleep %d========================",HEART_SLEEP_TM));
			sleep(HEART_SLEEP_TM);
		}
		
	}
    return 0;
}
static void send_transdtl(void* args)
{
	LOG((LOG_DEBUG,"����send_transdtl"));
	int ret = check_send_transdtl();
	if(ret)
	{
		LOG((LOG_DEBUG,"�������󣬼����˳��߳�"));
	}
	LOG((LOG_DEBUG,"����pthread_exit(0)"));
    pthread_exit(0);
}

static void open_send_transdtl_thread()
{
    pthread_t pid;
    int ret;
    ret = pthread_create(&pid, NULL, (void*)&send_transdtl, NULL);
    if (ret)
    {
        LOG((LOG_ERROR, "�����ϴ���ˮ�߳�ʧ��"));
        return;
    }
}


int main(int argc, char* const argv[])
{
    memset(&p16pos, 0, sizeof p16pos);
	memset(&p16card, 0, sizeof p16card);

    char temp[PATH_MAX + 1];
    realpath(argv[0], temp);
    strcpy(p16pos.base_dir, dirname(temp));
    // LOG((LOG_DEBUG, "����·��: %s", p16pos.base_dir));

    int8 ret;
    ret = check_config_db();
    if (ret)
    {
        error_exit(1, "��ʼ������ʧ��");
    }
	/*
	//����һ����������������
	p16_blackcard_t blackcard_t;
	blackcard_t.cardno = 184;
	blackcard_t.cardflag = 1;
	memcpy(blackcard_t.remark, "aaa", 3);
	blackcard_update_record(&blackcard_t);
	//
	*/
    ret = load_sys_config();
    if (ret)
    {
        error_exit(2, "���ز���ʧ��");
    }

    load_ip_config();
    // ��ʼ������
    do_init_all_hardware();
    // ����������Ϣ
    do_load_config();

    clear_transdtl();

    test_printer();
    do_check_desktop();

    p16pos.app_running = 1;
    show_statusbar();
	//�����ϴ���ˮ�߳�
	open_send_transdtl_thread();

    // ��ʾ���˵�
	init_purchase_menu();

    return 0;
}
// vim:foldmethod=marker

