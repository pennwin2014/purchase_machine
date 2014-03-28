/******************************************************************************
* Copyright SupWisdom 2012 all right reserved
* Module: ��ֵ��ҵ������
* File: deposit.c
* Auth: Cheng Tang<cheng.tang@supwisdom.com>
******************************************************************************/
#include <unistd.h>

#include "config.h"
#include "p16log.h"
#include "cardop.h"
#include "utils.h"
#include "menu.h"
#include "service.h"
#include "syspara.h"

#include "service.h"
#define DO_BEEP 1
#define NO_BEEP 0

// {{{ LED ��ʾ����
/**
 * convert money on led
 * @param money - money
 * @param buf - output buffer
 */
/* static void money_2_led_buffer(int money, uint8* buf) */
/* { */
/*     money = money % 1000000; */
/*     memset(buf, 0, 6); */
/*     if(money >= 0) */
/*     { */
/*         int t = money / 100; */
/*         char str[9]; */
/*         sprintf(str, "%d", t); */
/*         int p = strlen(str); */
/*         dis_buf_tran(str, p); */
/*         str[p - 1] += 0x80; */
/*         memcpy(buf, str, p); */

/*         t = money % 100; */
/*         sprintf(str, "%d", t); */
/*         dis_buf_tran(str, 2); */
/*         memcpy(buf + p, str, 2); */
/*     } */
/* } */

/* static void led_display(int money1, int money2) */
/* { */
/*     uint8 buffer[12]; */
/*     memset(buffer, 0, sizeof buffer); */
/*     money_2_led_buffer(money1, buffer); */
/*     money_2_led_buffer(money2, buffer + 6); */
/*     led_updat(buffer); */
/* } */
// }}}

// {{{ ��ӡ����
int pos_printer(p16_transdtl_t* transdtl, int page, int pagecnt)
{
    CLEAR_SCREEN;
    if(NULL != PRINTER)
    {
        ds_destory(PRINTER);
        PRINTER = ds_init(FS_DOT_NUM, 640 , BIT_L);
    }


    int y_coor = 50;
    int x_coor = 10;
    int y_offset = 36;
    char tip[64];

    dis_mod_set(PRINTER , HZ_16DZ , ASC_16DZ);
    sprintf(tip, "%d/%dҳ", page, pagecnt);
    printf_str(PRINTER, 300, 35, tip, 1);

    dis_mod_set(PRINTER , HZ_32DZ , ASC_32DZ);
    printf_str(PRINTER, 80, 1, "һ��ͨ�ֽ��ֵ", 1);


    sprintf(tip, "�ն˺�: %s", transdtl->devphyid);
    printf_str(PRINTER, x_coor, y_coor, tip, 1);
    y_coor += y_offset;

    sprintf(tip, "�ն���ˮ��: %d", transdtl->devseqno);
    printf_str(PRINTER, x_coor, y_coor, tip, 1);
    y_coor += y_offset;

    sprintf(tip, "����: %s", transdtl->transdate);
    printf_str(PRINTER, x_coor, y_coor, tip, 1);
    y_coor += y_offset;

    sprintf(tip, "ʱ��: %s", transdtl->transtime);
    printf_str(PRINTER, x_coor, y_coor, tip, 1);
    y_coor += y_offset;

    sprintf(tip, "����: %d", transdtl->cardno);
    printf_str(PRINTER, x_coor, y_coor, tip, 1);
    y_coor += y_offset;

    sprintf(tip, "��ֵ���: %.2f", transdtl->dpsamt / 100.0);
    printf_str(PRINTER, x_coor, y_coor, tip, 1);
    y_coor += y_offset;

    if(transdtl->amount - transdtl->dpsamt > 0)
    {
        sprintf(tip, "��������: %.2f", (transdtl->amount - transdtl->dpsamt) / 100.0);
        printf_str(PRINTER, x_coor, y_coor, tip, 1);
        y_coor += y_offset;
    }

    sprintf(tip, "���: %.2f", (transdtl->cardbefbal + transdtl->dpsamt) / 100.0);
    printf_str(PRINTER, x_coor, y_coor, tip, 1);
    y_coor += y_offset;

    sprintf(tip, "����Ա��: %d", transdtl->operid);
    printf_str(PRINTER, x_coor, y_coor, tip, 1);
    y_coor += y_offset;

    sprintf(tip, "��Ȩ��: %d", transdtl->authcode);
    printf_str(PRINTER, x_coor, y_coor, tip, 1);
    y_coor += y_offset;

    sprintf(tip, "���κ�: %d", transdtl->batchno);
    printf_str(PRINTER, x_coor, y_coor, tip, 1);
    y_coor += y_offset;

    sprintf(tip, "�ο���:\n %s", transdtl->refno);
    printf_str(PRINTER, x_coor, y_coor, tip, 1);
    y_coor += y_offset * 2 - 8;

    plan_dst*  plan;
    plan = plan_init(1);
    if(plan == NULL)
    {
        disp_msg("��ʼ����ӡ�ؼ�ʧ��", 10);
        return -1;
    }

    fs_print_dat(PRINTER->ptr, PRINTER->buf_size) ;
    int ret;
    uint8 step, key_val;
    while(1)
    {
        ret = fs_print_status(&step);
        if(0 == ret)
        {
            plan_step_dis(MAIN_DISP, plan, lcd_160_upd , 100) ; //��ӡ������
            dis_mod_set(MAIN_DISP , DEFAULT_HZ_FONT , DEFAULT_ASC_FONT);
            printf_str(MAIN_DISP, DISP_X_COOR + 8, DISP_Y_COOR + 50, "��ӡ���!", 1);
            lcd_160_upd() ;
            ret = 0;
            break;
        }
        else if(-1 == ret)
        {
            plan_step_dis(MAIN_DISP, plan, lcd_160_upd , step) ;  //��ӡ������
            dis_mod_set(MAIN_DISP , DEFAULT_HZ_FONT , DEFAULT_ASC_FONT);
            printf_str(MAIN_DISP, DISP_X_COOR + 8, DISP_Y_COOR + 50, "����ӡֽ", 1);
            lcd_160_upd();
            while(1)                  //��������
            {
                ret = get_key(&key_val, BLOCK_MOD , 1) ;
                if(ret == 0)                 //���˼�
                {
                    if(ESC_KEY == key_val) break;
                }
            }
            ret = -1;
            break;
        }
        else
        {
            plan_step_dis(MAIN_DISP, plan, lcd_160_upd , step) ;  //��ӡ������
            lcd_160_upd();
            continue;
        }
    }
    plan_destory(plan);
    //get_key( &key_val, BLOCK_MOD , 1 ) ;
    return ret;
}
// }}}
//
static int8 do_request_card(int is_beep, uint8 cardphyid[8], uint8* cardtype)
{
    int8 ret;
    uint32 tick = 0;
    CLEAR_SCREEN;
    uint32 tick2 = get_time_tick();
    // if(tick != tick2)
    {
        //LOG( ( LOG_DEBUG, "tick = %d", tick2 ) );
        CLEAR(MAIN_DISP, DISP_X_COOR, DISP_Y_COOR, DISP_X_PIXEL + DISP_X_COOR, DISP_Y_COOR + 120);
        dis_mod_set(MAIN_DISP , DEFAULT_HZ_FONT, DEFAULT_ASC_FONT);
        if(!is_beep)
            printf_str(MAIN_DISP, DISP_X_COOR + 30, DISP_Y_COOR + 10, "��ſ�...", 1) ;
        else
            printf_str(MAIN_DISP, DISP_X_COOR + 30, DISP_Y_COOR + 10, "��ȷ�Ϸſ�", 1);
        dis_mod_set(MAIN_DISP , INPUT_HZ_FONT , INPUT_ASC_FONT) ;
        printf_str(MAIN_DISP, DISP_X_COOR + 20, DISP_Y_COOR + 80, "��<ȡ��>�˳�" , 1);
        lcd_160_upd();

    }
    while(1)
    {
        ret = request_card_and_poweron(cardphyid, cardtype);
        if(0 == ret)
        {
            // memcpy(p16card.cardphyid, cardphyid , sizeof(p16card.cardphyid));
            // p16card.cardtype = cardtype;
            return 0;
        }
        tick2 = get_time_tick();
        if(tick != tick2)
        {
            tick = tick2;
            if(is_beep)
                beep(20);
        }
        uint8 keyval;
        ret = get_key(&keyval, TIM_DLY_MOD, 30);
        if(0 == ret)
        {
            if(ESC_KEY == keyval)
            {
                return 1;
            }
        }
    }
    return -1;
}
typedef struct
{
    p16_card_context* cardctx;
    p16_transdtl_t* transdtl;
} write_card_param_t;

static int do_retry_write_purchase_card(p16_card_context* cardctx,
                               p16_transdtl_t* transdtl,
                               size_t timeout)
{
    int ret;
    // ���³�ֵ��ʼ��

    char emsg[512] = {0};
    ret = read_user_card(cardctx, emsg);
    if(ret)
        return -1;
    ret = init_4_purchase(cardctx);
    if(ret)
        return -1;
    encode_hex(cardctx->random_num, 4, transdtl->random);
	ret = psam_purchase_mac(cardctx);
	if(ret)
		return -1;
    encode_hex(cardctx->mac1, 4, transdtl->mac1);
    SAFE_STR_CPY(cardctx->hostdate, transdtl->hostdate);
    SAFE_STR_CPY(cardctx->hosttime, transdtl->hosttime);
    return 0;

}

/*
static int do_retry_write_card(p16_card_context* cardctx,
                               p16_transdtl_t* transdtl,
                               size_t timeout)
{
    int ret;
    // ���³�ֵ��ʼ��

    char emsg[512] = {0};
    ret = read_user_card(cardctx, emsg);
    if(ret)
        return -1;
    ret = init_4_purchase(cardctx);
    if(ret)
        return -1;
    encode_hex(cardctx->random_num, 4, transdtl->random);
	ret = psam_purchase_mac(cardctx);
	if(ret)
		return -1;
    encode_hex(cardctx->mac1, 4, transdtl->mac1);
    SAFE_STR_CPY(cardctx->hostdate, transdtl->hostdate);
    SAFE_STR_CPY(cardctx->hosttime, transdtl->hosttime);
    return 0;

}
*/
static int do_write_card(p16_card_context* cardctx,
                         p16_transdtl_t* transdtl,
                         size_t timeout)
{
    uint8 cardphyid[8], cardtype;
    int write_card_ret, ret;
    // uint32 start, end, tick;
    // start = get_time_tick();
    // end = start;
    // tick = 0;
    while(1)
    {
        memset(cardphyid, 0 , sizeof cardphyid);
        ret = do_request_card(1, cardphyid, &cardtype);
        if(ret == 1)
        {
            disp_msg("�û�ȡ��д��,��ϵͳ�Ѽ���", 3);
            return 1;
        }
        else if(ret == -1)
            continue;
        if(memcmp(cardphyid, cardctx->cardphyid, sizeof cardphyid) != 0)
        {
            disp_msg("���ͬһ�ſ�", 2);
            continue;
        }
        // �ж�д���Ƿ�ɹ�
        ret = get_purchase_card_prove(cardctx);
        if(0 == ret)
            return 0; // ���ѳɹ�
        else if(-1 == ret)
            continue; // ����ʧ��
        else
        {
            LOG((LOG_DEBUG, "�ϴ�д�����ɹ�����д��!"));
            ret = do_retry_write_purchase_card(cardctx, transdtl, timeout);
            if(!ret)
            {
                write_card_ret = debit_4_purchase(cardctx);
                if(0 == write_card_ret)
                {
                    //disp_update_message(context, "д���ɹ�");
                    beep(80);
                    return 0;
                }
                disp_msg("����д��ʧ��", 2);
            }
        }
    }
    return -1;
}
static int do_write_card_proxy(void* userp,
                               uint8 timeout)
{
    write_card_param_t* param = (write_card_param_t*)userp;
    int ret, write_card_ret;

    CLEAR_SCREEN;
    printf_str(MAIN_DISP, DISP_X_COOR, DISP_Y_COOR, "�ȴ�д��..." , 1);
    lcd_160_upd();
    write_card_ret = debit_4_purchase(param->cardctx);
    // LOG((LOG_DEBUG, "do_write_card_proxy ok"));
    if(0 == write_card_ret)
    {
        //(context, "д���ɹ�");
        ret = 0;
    }
    else //if(1 == write_card_ret)
    {
        ret = do_write_card(param->cardctx, param->transdtl, timeout);
    }
    return ret;
}

static int write_card_timeout(p16_card_context* cardctx, p16_transdtl_t* transdtl,
                              size_t timeout)
{
    write_card_param_t param;
    param.cardctx = cardctx;
    param.transdtl = transdtl;
    return do_write_card_proxy(&param, timeout);
}

static int on_deposit_timeout(void* userp, uint8 timeout, uint8 est)
{
    p16_transdtl_t* transdtl = (p16_transdtl_t*)userp;
    int ret;
    ret = svc_deposit_query(transdtl);
    if(ret == SVC_OK)
        return 0;
    if(ret == SVC_FAILED)
    {
        char tip[280];
        sprintf(tip, "��ֵʧ��,%d\n %s", transdtl->errcode, transdtl->errmsg);
        disp_msg(tip, 10);
    }
    return -1;
}

static int8 do_calc_dpsamt(const p16_card_context* cardctx, uint32 money, uint32* dpsamt)
{
    syspara_feerate_t feerate;
    memset(&feerate, 0, sizeof feerate);
    int8 ret;
    ret = syspara_get_feerate(cardctx->feetype, &feerate);
    if(ret)
    {
        if(DB_NOTFOUND == ret)
            return 1;
        return -1;
    }
    *dpsamt = (uint32)(money * (100.0 - feerate.feerate) / 100.0);
    return 0;
}

static int do_card_purchase(uint32 money)
{
    int ret;
    dis_mod_set(MAIN_DISP , DEFAULT_HZ_FONT, DEFAULT_ASC_FONT);
    /* LOG((LOG_DEBUG, "Ѱ���ɹ�������")); */
    char emsg[257];
    uint8 cardphyid[8], cardtype;
    memset(cardphyid, 0 , sizeof cardphyid);
	//����0:cpu����Ѱ�����ϵ�
    ret = do_request_card(DO_BEEP, cardphyid, &cardtype);
    if(ret)
    {
        disp_msg("����ʧ�ܣ����ܿ۷�", 3);
        return -1;
    }
    if(memcmp(cardphyid, p16card.cardphyid, sizeof(cardphyid)) != 0)
    {
        disp_msg("��ʹ��ͬһ�ſ�", 8);
        return -1;
    }
    memset(emsg, 0, sizeof emsg);
    // ����1����ȡ��������Ϣ(���а���ѡĿ¼)
    p16_card_context card_ctx;
    memcpy(&card_ctx, &p16card, sizeof card_ctx);
    ret = read_user_purchase_card(&card_ctx, emsg);
    if(ret)
    {
        LOG((LOG_ERROR, "����ʧ��"));
        disp_msg(emsg, 3);
        return ret;
    }
    if(p16card.cardbefbal != card_ctx.cardbefbal ||
            p16card.dpscnt != card_ctx.dpscnt)
    {
        disp_msg("������", 3);
        return -1;
    }

    memset(emsg, 0, sizeof emsg);
    SAFE_GET_DATETIME("%Y%m%d%H%M%S", emsg);
    strncpy(p16card.termdate, emsg, 8);
    strncpy(p16card.termtime, emsg + 8, 6);
    uint32 payamt = money;
	//����3:���ѳ�ʼ��
    p16card.payamt = payamt; // ʵ�����ѽ��
    LOG((LOG_DEBUG, "ʵ�ʿ۷ѽ��=%d", p16card.payamt));
    ret = init_4_purchase(&p16card);
    if(ret)
    {
        disp_msg("��Ƭ���ѳ�ʼ��ʧ��" , 5);
        return ret;
    }
    // ����4:��psam������mac1
    ret = psam_purchase_mac(&p16card);
    if(ret)
    {
        disp_msg("��psam������mac1ʧ��" , 5);
        return ret;
    }
	/*
    // ����5:����ȷ��
    ret = debit_4_purchase(&p16card);
    if(ret)
    {
        disp_msg("��Ƭ����ȷ��ʧ��" , 5);
        return ret;
    }
	*/
	
    //����6: ��¼��ˮ
    p16_transdtl_t transdtl;
    memset(&transdtl, 0, sizeof transdtl);
    transdtl.cardno = p16card.cardno;
    transdtl.operid = p16pos.operid;
    SAFE_STR_CPY(transdtl.transdate, p16card.termdate);
    SAFE_STR_CPY(transdtl.transtime, p16card.termtime);
    fun_bcd_ascii(p16pos.termno, transdtl.termno, 6);
    encode_hex(p16card.cardphyid, 4, transdtl.cardphyid);
    SAFE_STR_CPY(transdtl.devphyid, p16pos.devphyid);
    transdtl.cardbefbal = p16card.cardbefbal;
    transdtl.paycnt = p16card.paycnt;
    transdtl.dpscnt = p16card.dpscnt;
    transdtl.amount = money;
   // transdtl.dpsamt = p16card.payamt; // ʵ�ʿ۷ѽ��
    transdtl.confirm = 0;
    transdtl.transflag = TRANS_INIT;
    transdtl.devseqno = p16pos.termseqno;
    transdtl.authcode = p16pos.authcode;
    transdtl.batchno = p16pos.batchno;
    transdtl.termid = p16pos.termid;
    encode_hex(p16card.random_num, 4, transdtl.random);
    encode_hex(p16card.mac1, 4, transdtl.mac1);

    ret = trans_add_record(&transdtl);
    if(ret)
    {
        disp_msg("�豸����,����ϵ����Ա", 10);
        return -1;
    }
	//����7: д��
    SAFE_STR_CPY(p16card.hostdate, transdtl.hostdate);
    SAFE_STR_CPY(p16card.hosttime, transdtl.hosttime);
    int write_card_ret = write_card_timeout(&p16card , &transdtl , 45);
    if(0 == write_card_ret)
    {
        transdtl.transflag = TRANS_WRITE_CARD_OK;
    }
    else
    {
        transdtl.transflag = TRANS_WRITE_CARD_FAILED;
    }
    encode_hex(p16card.tac, 4, transdtl.tac);
    ret = trans_update_record(&transdtl);
    if(ret)
    {
        beep(80);
        disp_msg("�豸����, ����ϵ����Ա", 10);
        return -1;
    }
    /*
    //����8:��ӡ
    if(0 == write_card_ret)
    {
        beep(30);
        uint8 pages = 0;
        uint8 keys[] = {ESC_KEY, ENT_KEY, 0};
        while(pages++ < p16pos.printer_pages)
        {
            ret = pos_printer(&transdtl, pages, p16pos.printer_pages);
            CLEAR(MAIN_DISP, 0, 0, SCREEN_X_PIXEL, SCREEN_Y_PIXEL);
            if(ret == 0)
            {
                if(p16pos.printer_pages == 1)
                    break;
                if(pages < p16pos.printer_pages)
                    confirm_info("����������ӡ��", keys);
            }
            else
            {
                disp_msg("��ӡʧ��!", 5);
            }
        }
        sprintf(emsg, "�ɹ�,���: %.2fԪ", (transdtl.cardbefbal + transdtl.dpsamt) / 100.0);
        // set_disp_msg_font(HZ_24DZ, ASC_24DZ);
        disp_msg(emsg, 5);
    }
    else
    {
        beep(80);
        disp_msg("��ֵд��ʧ��, �뵽�����Ĵ���", 15);
    }
	*/
    return 0;
}

static int do_card(uint32 money)
{
    int ret;
    dis_mod_set(MAIN_DISP , DEFAULT_HZ_FONT, DEFAULT_ASC_FONT);
    /* LOG((LOG_DEBUG, "Ѱ���ɹ�������")); */
    char emsg[257];
    uint8 cardphyid[8], cardtype;
    memset(cardphyid, 0 , sizeof cardphyid);
    ret = do_request_card(DO_BEEP, cardphyid, &cardtype);
    if(ret)
    {
        disp_msg("����ʧ�ܣ����ܳ�ֵ", 3);
        return -1;
    }
    if(memcmp(cardphyid, p16card.cardphyid, sizeof(cardphyid)) != 0)
    {
        disp_msg("��ʹ��ͬһ�ſ�", 8);
        return -1;
    }
    memset(emsg, 0, sizeof emsg);
    // ��ȡ��������Ϣ
    p16_card_context card_ctx;
    memcpy(&card_ctx, &p16card, sizeof card_ctx);
    ret = read_user_card(&card_ctx, emsg);
    if(ret)
    {
        LOG((LOG_ERROR, "����ʧ��"));
        disp_msg(emsg, 3);
        return ret;
    }
    if(p16card.cardbefbal != card_ctx.cardbefbal ||
            p16card.dpscnt != card_ctx.dpscnt)
    {
        disp_msg("������", 3);
        return -1;
    }

    memset(emsg, 0, sizeof emsg);
    SAFE_GET_DATETIME("%Y%m%d%H%M%S", emsg);
    strncpy(p16card.termdate, emsg, 8);
    strncpy(p16card.termtime, emsg + 8, 6);
    uint32 dpsamt = money;
    ret = do_calc_dpsamt(&p16card, money, &dpsamt);
    if(ret)
    {
        if(ret == -1)
        {
            disp_msg("���ܶ�ȡ�շѷ��ʣ�����ϵ����Ա", 3);
        }
        else
        {
            disp_msg("�ÿ���������ֵ", 4);
        }
        return -1;
    }
    p16card.dpsamt = dpsamt; // ʵ�ʳ�ֵ���
    LOG((LOG_DEBUG, "ʵ�ʳ�ֵ���=%d", p16card.dpsamt));
    ret = init_4_load(&p16card);
    if(ret)
    {
        disp_msg("��Ƭ��ֵ��ʼ��ʧ��" , 10);
        return ret;
    }
    // ��¼��ˮ
    p16_transdtl_t transdtl;
    memset(&transdtl, 0, sizeof transdtl);
    transdtl.cardno = p16card.cardno;
    transdtl.operid = p16pos.operid;
    SAFE_STR_CPY(transdtl.transdate, p16card.termdate);
    SAFE_STR_CPY(transdtl.transtime, p16card.termtime);
    fun_bcd_ascii(p16pos.termno, transdtl.termno, 6);
    encode_hex(p16card.cardphyid, 4, transdtl.cardphyid);
    SAFE_STR_CPY(transdtl.devphyid, p16pos.devphyid);
    transdtl.cardbefbal = p16card.cardbefbal;
    transdtl.paycnt = p16card.paycnt;
    transdtl.dpscnt = p16card.dpscnt;
    transdtl.amount = money;
    transdtl.dpsamt = p16card.dpsamt; // ʵ�ʳ�ֵ���
    transdtl.confirm = 0;
    transdtl.transflag = TRANS_INIT;
    transdtl.devseqno = p16pos.termseqno;
    transdtl.authcode = p16pos.authcode;
    transdtl.batchno = p16pos.batchno;
    transdtl.termid = p16pos.termid;
    encode_hex(p16card.random_num, 4, transdtl.random);
    encode_hex(p16card.mac1, 4, transdtl.mac1);

    ret = trans_add_record(&transdtl);
    if(ret)
    {
        disp_msg("�豸����,����ϵ����Ա", 10);
        return -1;
    }

    // �����̨
    ret = svc_deposit(&transdtl);
    // LOG((LOG_DEBUG,"svc_deposit ret=%d",ret));
    if(ret != SVC_OK)
    {
        beep(80);
        if(ret == SVC_FAILED)
        {
            char tip[280];
            sprintf(tip, "��ֵ��Ч,%d,%s", transdtl.errcode, transdtl.errmsg);
            disp_msg(tip, 10);

            transdtl.transflag = TRANS_SYS_FAILED;
            ret = trans_update_record(&transdtl);
            if(ret)
            {
                LOG((LOG_ERROR, "������ˮʧ��"));
                disp_msg("�豸����, ����ϵ����Ա", 10);
            }
            else
            {
                //disp_msg( "�����̨��ֵʧ��", 10 );
            }

            return -1;
        }
        else
        {
            // ��ѯ��ˮ
            ret = disp_msg_callback("��̨���׳�ʱ, �ȴ���ѯ", 30, &on_deposit_timeout, &transdtl);
            if(ret)
            {
                LOG((LOG_ERROR, "svc_deposit error,ret=%d", ret));
                disp_msg("���׳�ʱ, ����ϵ����Ա����", 15);
                return -1;
            }
        }
    }

    // д��
    decode_hex(transdtl.mac2, 8, p16card.mac2);
    SAFE_STR_CPY(p16card.hostdate, transdtl.hostdate);
    SAFE_STR_CPY(p16card.hosttime, transdtl.hosttime);
    int write_card_ret = write_card_timeout(&p16card , &transdtl , 45);
    if(0 == write_card_ret)
    {
        transdtl.transflag = TRANS_WRITE_CARD_OK;
    }
    else
    {
        transdtl.transflag = TRANS_WRITE_CARD_FAILED;
    }
    encode_hex(p16card.tac, 4, transdtl.tac);
    ret = trans_update_record(&transdtl);
    if(ret)
    {
        beep(80);
        disp_msg("�豸����, ����ϵ����Ա", 10);
        return -1;
    }
    // ��̨ȷ��
    while(1)
    {
        ret = svc_deposit_confirm(&transdtl, & (p16pos.termseqno));
        if(ret != SVC_OK)
        {
            if(ret == SVC_FAILED)
            {
                char tip[280];
                sprintf(tip, "ȷ��ʧ��,%d\n %s", transdtl.errcode, transdtl.errmsg);
                disp_msg(tip, 10);
                transdtl.confirm = 1;
            }
            else
            {
                LOG((LOG_DEBUG, "��ˮȷ�ϳ�ʱ"));
                sleep(1);
                continue;
            }
        }
        else
        {
            transdtl.confirm = 1;
        }

        ret = trans_update_record(&transdtl);
        if(ret)
        {
            beep(80);
            disp_msg("�豸����, ����ϵ����Ա", 10);
            return -1;
        }
        break;
    }
    if(0 == write_card_ret)
    {
        beep(30);
        uint8 pages = 0;
        uint8 keys[] = {ESC_KEY, ENT_KEY, 0};
        while(pages++ < p16pos.printer_pages)
        {
            ret = pos_printer(&transdtl, pages, p16pos.printer_pages);
            CLEAR(MAIN_DISP, 0, 0, SCREEN_X_PIXEL, SCREEN_Y_PIXEL);
            if(ret == 0)
            {
                if(p16pos.printer_pages == 1)
                    break;
                if(pages < p16pos.printer_pages)
                    confirm_info("����������ӡ��", keys);
            }
            else
            {
                disp_msg("��ӡʧ��!", 5);
            }
        }
        sprintf(emsg, "�ɹ�,���: %.2fԪ", (transdtl.cardbefbal + transdtl.dpsamt) / 100.0);
        // set_disp_msg_font(HZ_24DZ, ASC_24DZ);
        disp_msg(emsg, 5);
    }
    else
    {
        beep(80);
        disp_msg("��ֵд��ʧ��, �뵽�����Ĵ���", 15);
    }
    return 0;
}
// static proptey  money_input_property[] =
// {
//   {INPUT_HIN_STA ,  DISP_X_COOR , DISP_Y_COOR + 50},
//   {INPUT_RET_STA ,  DISP_X_COOR + 10, DISP_Y_COOR + 90},
//   {INPUT_RET_SIZ ,  INPUT_HEIGHT , 120},
//   {INPUT_HIND_FONT ,  DEFAULT_HZ_FONT, DEFAULT_ASC_FONT},
//   {INPUT_BCOR_SET , WHITE_COLR, 0},
//   {INPUT_MAX_LEN ,  8, 0},
//   {0xffff ,       0 , 0}
// } ;

static int deposit_check_transdtl()
{
    int ret;
    p16_transdtl_t transdtl;
    memset(&transdtl, 0, sizeof transdtl);
    ret = trans_get_last_record(&transdtl);
    if(ret == -1)
    {
        disp_msg("ϵͳ����,����ϵ����Ա", 10);
        return -1;
    }
    else if(ret == 0)
    {
        return 0;
    }
    CLEAR_SCREEN;
    printf_str(MAIN_DISP, DISP_X_COOR, DISP_Y_COOR, "�ȴ������쳣��ˮ...", 1);
    lcd_160_upd();
    if(transdtl.transflag == TRANS_INIT)
    {
        while(1)
        {
            p16_transdtl_t dtl;
            memset(&dtl, 0, sizeof dtl);
            memcpy(&dtl, &transdtl, sizeof dtl);
            dtl.operid = p16pos.operid;
            dtl.authcode = p16pos.authcode;
            dtl.batchno = p16pos.batchno;
            ret = svc_deposit_query(&dtl);
            LOG((LOG_DEBUG, "svc_deposit_query,ret=%d", ret));
            if(ret != SVC_OK)
            {
                if(ret == SVC_TIMEOUT)
                {
                    disp_msg("����쳣��ˮ��ʱ", 3);
                    continue;
                }
                transdtl.transflag = TRANS_SYS_FAILED;
            }
            else
            {
                transdtl.transflag = TRANS_WRITE_CARD_FAILED;
            }
            SAFE_STR_CPY(transdtl.refno, dtl.refno);
            break;
        }

        ret = trans_update_record(&transdtl);
        if(ret)
        {
            disp_msg("ϵͳ����, ����ϵ����Ա", 10);
            return -1;
        }
    }
    if(transdtl.transflag == TRANS_SYS_FAILED)
        return 0;
    int nextseqno;
    while(1)
    {
        ret = svc_deposit_confirm(&transdtl, &nextseqno);
        if(ret)
        {
            disp_msg("ȷ�Ͻ�����ˮʧ��", 3);
        }
        else
            break;
    }
    transdtl.confirm = 1;
    ret = trans_update_record(&transdtl);
    if(ret)
    {
        disp_msg("ϵͳ����, ����ϵ����Ա", 10);
        return -1;
    }

    return 1;
}
static int do_waiting_purchase_card(uint8* cardphyid)
{
    int ret = do_request_card(0, p16card.cardphyid, &(p16card.cardtype));
    if(ret)
        return -1;
    char emsg[257];
    memset(emsg, 0, sizeof emsg);
    ret = read_user_purchase_card(&p16card, emsg);
    if(ret)
    {
        LOG((LOG_ERROR, "����ʧ��"));
        disp_msg(emsg, 1);
        return 1;
    }
    if(p16card.cardstatus != 0)
    {
        disp_msg("������", 3);
        return 1;
    }
    char current_date[21];
    uint8 hex_date[4];
    SAFE_GET_DATETIME("%Y%m%d", current_date);
    // LOG((LOG_DEBUG,"date=%s", current_date));
    decode_hex(current_date, 8, hex_date);
    // p16dump_hex(LOG_DEBUG, hex_date, 4);
    // p16dump_hex(LOG_DEBUG, p16card.expiredate, 4);
    if(memcmp(hex_date, p16card.expiredate, 4) > 0)
    {
        disp_msg("���ѹ���", 3);
        return 1;
    }
    memcpy(cardphyid, p16card.cardphyid, 4);
    return 0;
}

static int do_waiting_card(uint8* cardphyid)
{
    int ret = do_request_card(0, p16card.cardphyid, &(p16card.cardtype));
    if(ret)
        return -1;
    char emsg[257];
    memset(emsg, 0, sizeof emsg);
    ret = read_user_card(&p16card, emsg);
    if(ret)
    {
        LOG((LOG_ERROR, "����ʧ��"));
        disp_msg(emsg, 1);
        return 1;
    }
    if(p16card.cardstatus != 0)
    {
        disp_msg("������", 3);
        return 1;
    }
    char current_date[21];
    uint8 hex_date[4];
    SAFE_GET_DATETIME("%Y%m%d", current_date);
    // LOG((LOG_DEBUG,"date=%s", current_date));
    decode_hex(current_date, 8, hex_date);
    // p16dump_hex(LOG_DEBUG, hex_date, 4);
    // p16dump_hex(LOG_DEBUG, p16card.expiredate, 4);
    if(memcmp(hex_date, p16card.expiredate, 4) > 0)
    {
        disp_msg("���ѹ���", 3);
        return 1;
    }
    memcpy(cardphyid, p16card.cardphyid, 4);
    return 0;
}
static int do_input_money_ex(uint32* money)
{
    uint8 upd = 1;
    uint8 key;
    int32 ret;
    char value[20] = {0};
    char temp[20];
  
    const uint32 x_coor = DISP_X_COOR + 10;
    const uint32 y_coor =  DISP_Y_COOR + 90;
    printf_str(MAIN_DISP, x_coor, DISP_Y_COOR + 40, "������(Ԫ)", 1);
 
    while(1)
    {
        if(upd)
        {
            CLEAR(MAIN_DISP, x_coor, y_coor, x_coor + 120, y_coor + INPUT_HEIGHT);
            box(MAIN_DISP, x_coor, y_coor, x_coor + 120, y_coor + INPUT_HEIGHT, WHITE_COLR);
            rectangle(MAIN_DISP, x_coor, y_coor, x_coor + 120, y_coor + INPUT_HEIGHT, 1);
            printf_str(MAIN_DISP, x_coor + 2, y_coor, value, 1);
            lcd_160_upd();
            upd = 0;
        }
        ret = get_key(&key, TIM_DLY_MOD, 50);
        if(ret == 0)
        {
            // LOG((LOG_DEBUG, "key : %d", key));
            uint8 t;
            if(is_number_key(key, &t))
            {
                if(strlen(value) >= 6)
                    continue;
                strcpy(temp, value);
                sprintf(value, "%s%d", temp, t);
                upd = 1;
            }
            else if(key == ESC_KEY)
            {
                return -1;
            }
            else if(key == ENT_KEY)
            {
                *money = atoi(value) * 100;
                return 0;
            }
            else if(key == KEY_SHIFT)
            {
                uint8 vl = strlen(value);
                if(vl > 0)
                    value[vl - 1] = 0;
                upd = 1;
            }
        }
    }
}

static int do_input_money(uint32* money, const uint8* cardphyid)
{
    uint8 upd = 1;
    uint8 key;
    int32 ret;
    uint8 req_card_phyid[4], cardtype;
    char value[20] = {0};
    char temp[20];
    // uint32 tick, tick2;
    const uint32 x_coor = DISP_X_COOR + 10;
    const uint32 y_coor =  DISP_Y_COOR + 90;
    printf_str(MAIN_DISP, x_coor, DISP_Y_COOR + 40, "������(Ԫ)", 1);
    // tick2 = get_time_tick();
    while(1)
    {
        if(upd)
        {
            CLEAR(MAIN_DISP, x_coor, y_coor, x_coor + 120, y_coor + INPUT_HEIGHT);
            box(MAIN_DISP, x_coor, y_coor, x_coor + 120, y_coor + INPUT_HEIGHT, WHITE_COLR);
            rectangle(MAIN_DISP, x_coor, y_coor, x_coor + 120, y_coor + INPUT_HEIGHT, 1);
            printf_str(MAIN_DISP, x_coor + 2, y_coor, value, 1);
            lcd_160_upd();
            upd = 0;
        }
        // tick2 = get_time_tick();
        // if(tick != tick2)
        {
            // tick = tick2;
            ret = request_card(req_card_phyid, &cardtype);
            if(ret)
            {
                return -1;
            }
            if(memcmp(req_card_phyid, cardphyid, 4) != 0)
            {
                LOG((LOG_DEBUG, "not same card"));
                return -1;
            }
        }
        ret = get_key(&key, TIM_DLY_MOD, 50);
        if(ret == 0)
        {
            // LOG((LOG_DEBUG, "key : %d", key));
            uint8 t;
            if(is_number_key(key, &t))
            {
                if(strlen(value) >= 6)
                    continue;
                strcpy(temp, value);
                sprintf(value, "%s%d", temp, t);
                upd = 1;
            }
            else if(key == ESC_KEY)
            {
                return -1;
            }
            else if(key == ENT_KEY)
            {
                *money = atoi(value) * 100;
                return 0;
            }
            else if(key == KEY_SHIFT)
            {
                uint8 vl = strlen(value);
                if(vl > 0)
                    value[vl - 1] = 0;
                upd = 1;
            }
        }
    }
}
static void do_main_purchase_loop()
{
    int32 ret;
    char tip[64];
    while(1)
    {
		//1.���ݿ���Ϣ

		//2.��ȡ����Ϣ
		ret = do_waiting_purchase_card(p16card.cardphyid);
        if(ret == -1)
        {
            ret = confirm_dialog("�Ƿ��˳�������");
            if(!ret)
            {
                return;
            }
            continue;
        }
        else if(ret == 1)
        {
            continue;
        }
		//3.�������ѽ��
        CLEAR_SCREEN;
        sprintf(tip, "���%d.%02dԪ", p16card.cardbefbal / 100, p16card.cardbefbal % 100);
        dis_mod_set(MAIN_DISP , DEFAULT_HZ_FONT, DEFAULT_ASC_FONT);
        printf_str(MAIN_DISP, DISP_X_COOR, DISP_Y_COOR, tip, 1);
        lcd_160_upd();
		
        uint32 money;
        if(do_input_money_ex(&money) != 0)
            continue;

        LOG((LOG_DEBUG, "����۷ѽ��%d", money));
        if(money % 100 != 0 || money > 1000000 || money == 0)
        {
            disp_msg("������Ϸ�", 3);
            continue;
        }
		sprintf(tip, "�Ƿ�۷�%.2fԪ?", money / 100.0);
		ret = confirm_dialog(tip);
		if(!ret)
		{
			//4.�������ѵĽ��ִ�п۷Ѳ���
			ret = do_card_purchase(money);
			if(!ret)
				disp_msg("�۷ѳɹ�!!", 3);
			else
				disp_msg("�۷�ʧ��!!", 3);
		}
		else
		{
			disp_msg("�۷�ȡ��!!", 3);
		}
        LOG((LOG_DEBUG, "�������һ��"));
    }
}

static void do_main_loop()
{
    // input_dst* money_input;
    int32 ret;
    uint8 cardphyid[8];
    char tip[64];
    while(1)
    {
        if(p16pos.last_trans_timeout == 1)
        {
            while(1)
            {
                ret = deposit_check_transdtl();
                if(ret == 0)
                    break;
                else if(ret == 1)
                    continue;
                else
                    return;
            }
            p16pos.last_trans_timeout = 0;
        }
        ret = do_waiting_card(cardphyid);
        if(ret == -1)
        {
            ret = confirm_dialog("�Ƿ��˳�������");
            if(!ret)
            {
                return;
            }
            continue;
        }
        else if(ret == 1)
        {
            continue;
        }
        CLEAR_SCREEN;
        sprintf(tip, "���%d.%02dԪ", p16card.cardbefbal / 100, p16card.cardbefbal % 100);
        dis_mod_set(MAIN_DISP , DEFAULT_HZ_FONT, DEFAULT_ASC_FONT);
        printf_str(MAIN_DISP, DISP_X_COOR, DISP_Y_COOR, tip, 1);
        lcd_160_upd();

        p16pos.login_time_tick = get_time_tick();
        uint32 money;
        if(do_input_money(&money, cardphyid) != 0)
            continue;
        LOG((LOG_DEBUG, "������%d", money));
        if(money % 100 != 0 || money > 1000000 || money == 0)
        {
            disp_msg("������Ϸ�", 10);
            continue;
        }
        if(money >= 50000)   // ���ڵ��� 500
        {
            char tip[64];
            sprintf(tip, "�Ƿ��ֵ%.2fԪ?", money / 100.0);
            ret = confirm_dialog(tip);
            if(ret < 0)
            {
                continue;
            }
        }
        do_card(money);

        LOG((LOG_DEBUG, "�������һ��"));
    }
}


static int do_request_opercard(void* userp, uint8 timeout, uint8 est)
{
    int ret;
    unsigned char cardphyid[8];
    char cardtype;
    ret = request_card(cardphyid, &cardtype);
    if(!ret)
    {
        memcpy(userp, cardphyid, 4);
        return 0;
    }
    return -1;
}

// {{{ check_oper_login
int check_oper_login()
{
    if(p16pos.login_flag == 1)
        return 0;
    if(strlen(p16pos.devphyid) != DEFAULT_DEVPHYID_LEN)
    {
        disp_msg("���������ն˺�", 8);
        return -1;
    }
    if(svc_app_login())
        return -1;

    unsigned char oper_card[8];
    int ret;
    ret = disp_msg_callback("��ˢ����Ա��", 15, do_request_opercard, oper_card);
    if(ret)
    {
        return -1;
    }
    CLEAR_SCREEN;
    input_dst* user_input;
    user_input = input_init(PASSWORD_MOD);
    input_property_set(user_input, INPUT_HIND, "����");
    input_props_set(user_input, input_one_property);
    input_property_set(user_input, INPUT_MAX_LEN, 6);
    input_dis(MAIN_DISP, user_input);
    printf_str(MAIN_DISP, DISP_X_COOR, DISP_Y_COOR, "���������Ա������", 1);
    lcd_160_upd();

    ret = input_key_manege(MAIN_DISP, user_input , lcd_160_upd);
    if(ret < 0)
    {
        input_destory(user_input);
        return -1;
    }
    char operpwd[25] = {0};
    input_str_get(user_input, operpwd);
    input_destory(user_input);
    char oper_cardphyid[17];
    encode_hex(oper_card, 4, oper_cardphyid);
    int cfgverno;
    if(svc_oper_login(oper_cardphyid, operpwd, &cfgverno))
    {
        return -1;
    }
    if(p16pos.cfgverno != cfgverno)
    {
        // ���س�ֵ���ò���
        ret = svc_get_download_dps_para(p16pos.devphyid);
        if(ret)
        {
            return -1;
        }
        ret = update_config_int("sys.cfgverno", cfgverno);
        if(ret)
        {
            disp_msg("����ϵͳ�汾��ʧ��", 5);
            return -1;
        }
        p16pos.cfgverno = cfgverno;
    }
    p16pos.login_flag = 1;
    return 0;
}
//}}}
void purchase_main()
{
	struct pic_lst* main_menu_ptr;
	main_menu_ptr = piclist_init();
	if (NULL == main_menu_ptr)
	{
		error_exit(2, "��ʼ����Ļ����ʧ��");
	}
	jpg_display(MAIN_DISP, 0, 0, DESKTOP_JPG) ;  //gepspxs19.jpg //  ltby.jpg
	jpg_stormem(MAIN_DISP, MAIN_DISP->bak_ptr, 0, 0, DESKTOP_JPG);

	piclist_show(p16pos.main_disp, main_menu_ptr);
	//while(1)
	{
		CLEAR_SCREEN;
		piclist_show(MAIN_DISP, main_menu_ptr);
		lcd_160_upd();
		//
		do_main_purchase_loop();
		dc_reset(RFDEV, 0);
	}
}
// {{{ deposit_main
void deposit_main()
{
    if(p16pos.load_sam != 1)
    {
        disp_msg("�밲װPSAM����", 10);
        return;
    }
    // ������Ա�Ƿ��¼
    if(check_oper_login())
        return;
    int ret, count;
    count = 0;
    while(1)
    {
        ret = deposit_check_transdtl();
        if(ret == 0)
            break;
        if(ret == -1)
        {
            error_exit(3, "����ҵ��ʧ��");
        }
        ++count;
    }
    if(count > 0)
    {
        disp_msg("�����쳣��ˮ���", 5);
    }
    // �����ֵ����
    p16pos.last_trans_timeout = 0;
    do_main_loop();
    dc_reset(RFDEV, 0);
}
//}}}

