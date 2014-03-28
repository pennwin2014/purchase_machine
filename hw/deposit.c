/******************************************************************************
* Copyright SupWisdom 2012 all right reserved
* Module: 充值主业务流程
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

// {{{ LED 显示函数
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

// {{{ 打印函数
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
    sprintf(tip, "%d/%d页", page, pagecnt);
    printf_str(PRINTER, 300, 35, tip, 1);

    dis_mod_set(PRINTER , HZ_32DZ , ASC_32DZ);
    printf_str(PRINTER, 80, 1, "一卡通现金充值", 1);


    sprintf(tip, "终端号: %s", transdtl->devphyid);
    printf_str(PRINTER, x_coor, y_coor, tip, 1);
    y_coor += y_offset;

    sprintf(tip, "终端流水号: %d", transdtl->devseqno);
    printf_str(PRINTER, x_coor, y_coor, tip, 1);
    y_coor += y_offset;

    sprintf(tip, "日期: %s", transdtl->transdate);
    printf_str(PRINTER, x_coor, y_coor, tip, 1);
    y_coor += y_offset;

    sprintf(tip, "时间: %s", transdtl->transtime);
    printf_str(PRINTER, x_coor, y_coor, tip, 1);
    y_coor += y_offset;

    sprintf(tip, "卡号: %d", transdtl->cardno);
    printf_str(PRINTER, x_coor, y_coor, tip, 1);
    y_coor += y_offset;

    sprintf(tip, "充值金额: %.2f", transdtl->dpsamt / 100.0);
    printf_str(PRINTER, x_coor, y_coor, tip, 1);
    y_coor += y_offset;

    if(transdtl->amount - transdtl->dpsamt > 0)
    {
        sprintf(tip, "其它费用: %.2f", (transdtl->amount - transdtl->dpsamt) / 100.0);
        printf_str(PRINTER, x_coor, y_coor, tip, 1);
        y_coor += y_offset;
    }

    sprintf(tip, "余额: %.2f", (transdtl->cardbefbal + transdtl->dpsamt) / 100.0);
    printf_str(PRINTER, x_coor, y_coor, tip, 1);
    y_coor += y_offset;

    sprintf(tip, "操作员号: %d", transdtl->operid);
    printf_str(PRINTER, x_coor, y_coor, tip, 1);
    y_coor += y_offset;

    sprintf(tip, "授权号: %d", transdtl->authcode);
    printf_str(PRINTER, x_coor, y_coor, tip, 1);
    y_coor += y_offset;

    sprintf(tip, "批次号: %d", transdtl->batchno);
    printf_str(PRINTER, x_coor, y_coor, tip, 1);
    y_coor += y_offset;

    sprintf(tip, "参考号:\n %s", transdtl->refno);
    printf_str(PRINTER, x_coor, y_coor, tip, 1);
    y_coor += y_offset * 2 - 8;

    plan_dst*  plan;
    plan = plan_init(1);
    if(plan == NULL)
    {
        disp_msg("初始化打印控件失败", 10);
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
            plan_step_dis(MAIN_DISP, plan, lcd_160_upd , 100) ; //打印进度条
            dis_mod_set(MAIN_DISP , DEFAULT_HZ_FONT , DEFAULT_ASC_FONT);
            printf_str(MAIN_DISP, DISP_X_COOR + 8, DISP_Y_COOR + 50, "打印完成!", 1);
            lcd_160_upd() ;
            ret = 0;
            break;
        }
        else if(-1 == ret)
        {
            plan_step_dis(MAIN_DISP, plan, lcd_160_upd , step) ;  //打印进度条
            dis_mod_set(MAIN_DISP , DEFAULT_HZ_FONT , DEFAULT_ASC_FONT);
            printf_str(MAIN_DISP, DISP_X_COOR + 8, DISP_Y_COOR + 50, "检查打印纸", 1);
            lcd_160_upd();
            while(1)                  //按键返回
            {
                ret = get_key(&key_val, BLOCK_MOD , 1) ;
                if(ret == 0)                 //按了键
                {
                    if(ESC_KEY == key_val) break;
                }
            }
            ret = -1;
            break;
        }
        else
        {
            plan_step_dis(MAIN_DISP, plan, lcd_160_upd , step) ;  //打印进度条
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
            printf_str(MAIN_DISP, DISP_X_COOR + 30, DISP_Y_COOR + 10, "请放卡...", 1) ;
        else
            printf_str(MAIN_DISP, DISP_X_COOR + 30, DISP_Y_COOR + 10, "请确认放卡", 1);
        dis_mod_set(MAIN_DISP , INPUT_HZ_FONT , INPUT_ASC_FONT) ;
        printf_str(MAIN_DISP, DISP_X_COOR + 20, DISP_Y_COOR + 80, "按<取消>退出" , 1);
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
    // 重新充值初始化

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
    // 重新充值初始化

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
            disp_msg("用户取消写卡,但系统已记账", 3);
            return 1;
        }
        else if(ret == -1)
            continue;
        if(memcmp(cardphyid, cardctx->cardphyid, sizeof cardphyid) != 0)
        {
            disp_msg("请放同一张卡", 2);
            continue;
        }
        // 判断写卡是否成功
        ret = get_purchase_card_prove(cardctx);
        if(0 == ret)
            return 0; // 消费成功
        else if(-1 == ret)
            continue; // 读卡失败
        else
        {
            LOG((LOG_DEBUG, "上次写卡不成功，补写卡!"));
            ret = do_retry_write_purchase_card(cardctx, transdtl, timeout);
            if(!ret)
            {
                write_card_ret = debit_4_purchase(cardctx);
                if(0 == write_card_ret)
                {
                    //disp_update_message(context, "写卡成功");
                    beep(80);
                    return 0;
                }
                disp_msg("重试写卡失败", 2);
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
    printf_str(MAIN_DISP, DISP_X_COOR, DISP_Y_COOR, "等待写卡..." , 1);
    lcd_160_upd();
    write_card_ret = debit_4_purchase(param->cardctx);
    // LOG((LOG_DEBUG, "do_write_card_proxy ok"));
    if(0 == write_card_ret)
    {
        //(context, "写卡成功");
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
        sprintf(tip, "充值失败,%d\n %s", transdtl->errcode, transdtl->errmsg);
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
    /* LOG((LOG_DEBUG, "寻卡成功。。。")); */
    char emsg[257];
    uint8 cardphyid[8], cardtype;
    memset(cardphyid, 0 , sizeof cardphyid);
	//步骤0:cpu卡的寻卡和上电
    ret = do_request_card(DO_BEEP, cardphyid, &cardtype);
    if(ret)
    {
        disp_msg("读卡失败，不能扣费", 3);
        return -1;
    }
    if(memcmp(cardphyid, p16card.cardphyid, sizeof(cardphyid)) != 0)
    {
        disp_msg("请使用同一张卡", 8);
        return -1;
    }
    memset(emsg, 0, sizeof emsg);
    // 步骤1：读取卡基本信息(其中包括选目录)
    p16_card_context card_ctx;
    memcpy(&card_ctx, &p16card, sizeof card_ctx);
    ret = read_user_purchase_card(&card_ctx, emsg);
    if(ret)
    {
        LOG((LOG_ERROR, "读卡失败"));
        disp_msg(emsg, 3);
        return ret;
    }
    if(p16card.cardbefbal != card_ctx.cardbefbal ||
            p16card.dpscnt != card_ctx.dpscnt)
    {
        disp_msg("卡金额不符", 3);
        return -1;
    }

    memset(emsg, 0, sizeof emsg);
    SAFE_GET_DATETIME("%Y%m%d%H%M%S", emsg);
    strncpy(p16card.termdate, emsg, 8);
    strncpy(p16card.termtime, emsg + 8, 6);
    uint32 payamt = money;
	//步骤3:消费初始化
    p16card.payamt = payamt; // 实际消费金额
    LOG((LOG_DEBUG, "实际扣费金额=%d", p16card.payamt));
    ret = init_4_purchase(&p16card);
    if(ret)
    {
        disp_msg("卡片消费初始化失败" , 5);
        return ret;
    }
    // 步骤4:向psam卡请求mac1
    ret = psam_purchase_mac(&p16card);
    if(ret)
    {
        disp_msg("向psam卡请求mac1失败" , 5);
        return ret;
    }
	/*
    // 步骤5:消费确认
    ret = debit_4_purchase(&p16card);
    if(ret)
    {
        disp_msg("卡片消费确认失败" , 5);
        return ret;
    }
	*/
	
    //步骤6: 记录流水
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
   // transdtl.dpsamt = p16card.payamt; // 实际扣费金额
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
        disp_msg("设备故障,请联系管理员", 10);
        return -1;
    }
	//步骤7: 写卡
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
        disp_msg("设备故障, 请联系管理员", 10);
        return -1;
    }
    /*
    //步骤8:打印
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
                    confirm_info("按键继续打印！", keys);
            }
            else
            {
                disp_msg("打印失败!", 5);
            }
        }
        sprintf(emsg, "成功,余额: %.2f元", (transdtl.cardbefbal + transdtl.dpsamt) / 100.0);
        // set_disp_msg_font(HZ_24DZ, ASC_24DZ);
        disp_msg(emsg, 5);
    }
    else
    {
        beep(80);
        disp_msg("充值写卡失败, 请到卡中心处理", 15);
    }
	*/
    return 0;
}

static int do_card(uint32 money)
{
    int ret;
    dis_mod_set(MAIN_DISP , DEFAULT_HZ_FONT, DEFAULT_ASC_FONT);
    /* LOG((LOG_DEBUG, "寻卡成功。。。")); */
    char emsg[257];
    uint8 cardphyid[8], cardtype;
    memset(cardphyid, 0 , sizeof cardphyid);
    ret = do_request_card(DO_BEEP, cardphyid, &cardtype);
    if(ret)
    {
        disp_msg("读卡失败，不能充值", 3);
        return -1;
    }
    if(memcmp(cardphyid, p16card.cardphyid, sizeof(cardphyid)) != 0)
    {
        disp_msg("请使用同一张卡", 8);
        return -1;
    }
    memset(emsg, 0, sizeof emsg);
    // 读取卡基本信息
    p16_card_context card_ctx;
    memcpy(&card_ctx, &p16card, sizeof card_ctx);
    ret = read_user_card(&card_ctx, emsg);
    if(ret)
    {
        LOG((LOG_ERROR, "读卡失败"));
        disp_msg(emsg, 3);
        return ret;
    }
    if(p16card.cardbefbal != card_ctx.cardbefbal ||
            p16card.dpscnt != card_ctx.dpscnt)
    {
        disp_msg("卡金额不符", 3);
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
            disp_msg("不能读取收费费率，请联系管理员", 3);
        }
        else
        {
            disp_msg("该卡类别不允许充值", 4);
        }
        return -1;
    }
    p16card.dpsamt = dpsamt; // 实际充值金额
    LOG((LOG_DEBUG, "实际充值金额=%d", p16card.dpsamt));
    ret = init_4_load(&p16card);
    if(ret)
    {
        disp_msg("卡片充值初始化失败" , 10);
        return ret;
    }
    // 记录流水
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
    transdtl.dpsamt = p16card.dpsamt; // 实际充值金额
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
        disp_msg("设备故障,请联系管理员", 10);
        return -1;
    }

    // 请求后台
    ret = svc_deposit(&transdtl);
    // LOG((LOG_DEBUG,"svc_deposit ret=%d",ret));
    if(ret != SVC_OK)
    {
        beep(80);
        if(ret == SVC_FAILED)
        {
            char tip[280];
            sprintf(tip, "充值无效,%d,%s", transdtl.errcode, transdtl.errmsg);
            disp_msg(tip, 10);

            transdtl.transflag = TRANS_SYS_FAILED;
            ret = trans_update_record(&transdtl);
            if(ret)
            {
                LOG((LOG_ERROR, "更新流水失败"));
                disp_msg("设备故障, 请联系管理员", 10);
            }
            else
            {
                //disp_msg( "请求后台充值失败", 10 );
            }

            return -1;
        }
        else
        {
            // 查询流水
            ret = disp_msg_callback("后台交易超时, 等待查询", 30, &on_deposit_timeout, &transdtl);
            if(ret)
            {
                LOG((LOG_ERROR, "svc_deposit error,ret=%d", ret));
                disp_msg("交易超时, 请联系管理员对账", 15);
                return -1;
            }
        }
    }

    // 写卡
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
        disp_msg("设备故障, 请联系管理员", 10);
        return -1;
    }
    // 后台确认
    while(1)
    {
        ret = svc_deposit_confirm(&transdtl, & (p16pos.termseqno));
        if(ret != SVC_OK)
        {
            if(ret == SVC_FAILED)
            {
                char tip[280];
                sprintf(tip, "确认失败,%d\n %s", transdtl.errcode, transdtl.errmsg);
                disp_msg(tip, 10);
                transdtl.confirm = 1;
            }
            else
            {
                LOG((LOG_DEBUG, "流水确认超时"));
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
            disp_msg("设备故障, 请联系管理员", 10);
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
                    confirm_info("按键继续打印！", keys);
            }
            else
            {
                disp_msg("打印失败!", 5);
            }
        }
        sprintf(emsg, "成功,余额: %.2f元", (transdtl.cardbefbal + transdtl.dpsamt) / 100.0);
        // set_disp_msg_font(HZ_24DZ, ASC_24DZ);
        disp_msg(emsg, 5);
    }
    else
    {
        beep(80);
        disp_msg("充值写卡失败, 请到卡中心处理", 15);
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
        disp_msg("系统故障,请联系管理员", 10);
        return -1;
    }
    else if(ret == 0)
    {
        return 0;
    }
    CLEAR_SCREEN;
    printf_str(MAIN_DISP, DISP_X_COOR, DISP_Y_COOR, "等待处理异常流水...", 1);
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
                    disp_msg("检查异常流水超时", 3);
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
            disp_msg("系统故障, 请联系管理员", 10);
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
            disp_msg("确认交易流水失败", 3);
        }
        else
            break;
    }
    transdtl.confirm = 1;
    ret = trans_update_record(&transdtl);
    if(ret)
    {
        disp_msg("系统故障, 请联系管理员", 10);
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
        LOG((LOG_ERROR, "读卡失败"));
        disp_msg(emsg, 1);
        return 1;
    }
    if(p16card.cardstatus != 0)
    {
        disp_msg("卡已锁", 3);
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
        disp_msg("卡已过期", 3);
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
        LOG((LOG_ERROR, "读卡失败"));
        disp_msg(emsg, 1);
        return 1;
    }
    if(p16card.cardstatus != 0)
    {
        disp_msg("卡已锁", 3);
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
        disp_msg("卡已过期", 3);
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
    printf_str(MAIN_DISP, x_coor, DISP_Y_COOR + 40, "输入金额(元)", 1);
 
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
    printf_str(MAIN_DISP, x_coor, DISP_Y_COOR + 40, "输入金额(元)", 1);
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
		//1.数据库信息

		//2.读取卡信息
		ret = do_waiting_purchase_card(p16card.cardphyid);
        if(ret == -1)
        {
            ret = confirm_dialog("是否退出操作？");
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
		//3.输入消费金额
        CLEAR_SCREEN;
        sprintf(tip, "余额%d.%02d元", p16card.cardbefbal / 100, p16card.cardbefbal % 100);
        dis_mod_set(MAIN_DISP , DEFAULT_HZ_FONT, DEFAULT_ASC_FONT);
        printf_str(MAIN_DISP, DISP_X_COOR, DISP_Y_COOR, tip, 1);
        lcd_160_upd();
		
        uint32 money;
        if(do_input_money_ex(&money) != 0)
            continue;

        LOG((LOG_DEBUG, "输入扣费金额%d", money));
        if(money % 100 != 0 || money > 1000000 || money == 0)
        {
            disp_msg("输入金额不合法", 3);
            continue;
        }
		sprintf(tip, "是否扣费%.2f元?", money / 100.0);
		ret = confirm_dialog(tip);
		if(!ret)
		{
			//4.根据消费的金额执行扣费操作
			ret = do_card_purchase(money);
			if(!ret)
				disp_msg("扣费成功!!", 3);
			else
				disp_msg("扣费失败!!", 3);
		}
		else
		{
			disp_msg("扣费取消!!", 3);
		}
        LOG((LOG_DEBUG, "处理完成一次"));
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
            ret = confirm_dialog("是否退出操作？");
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
        sprintf(tip, "余额%d.%02d元", p16card.cardbefbal / 100, p16card.cardbefbal % 100);
        dis_mod_set(MAIN_DISP , DEFAULT_HZ_FONT, DEFAULT_ASC_FONT);
        printf_str(MAIN_DISP, DISP_X_COOR, DISP_Y_COOR, tip, 1);
        lcd_160_upd();

        p16pos.login_time_tick = get_time_tick();
        uint32 money;
        if(do_input_money(&money, cardphyid) != 0)
            continue;
        LOG((LOG_DEBUG, "输入金额%d", money));
        if(money % 100 != 0 || money > 1000000 || money == 0)
        {
            disp_msg("输入金额不合法", 10);
            continue;
        }
        if(money >= 50000)   // 大于等于 500
        {
            char tip[64];
            sprintf(tip, "是否充值%.2f元?", money / 100.0);
            ret = confirm_dialog(tip);
            if(ret < 0)
            {
                continue;
            }
        }
        do_card(money);

        LOG((LOG_DEBUG, "处理完成一次"));
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
        disp_msg("请先设置终端号", 8);
        return -1;
    }
    if(svc_app_login())
        return -1;

    unsigned char oper_card[8];
    int ret;
    ret = disp_msg_callback("请刷操作员卡", 15, do_request_opercard, oper_card);
    if(ret)
    {
        return -1;
    }
    CLEAR_SCREEN;
    input_dst* user_input;
    user_input = input_init(PASSWORD_MOD);
    input_property_set(user_input, INPUT_HIND, "密码");
    input_props_set(user_input, input_one_property);
    input_property_set(user_input, INPUT_MAX_LEN, 6);
    input_dis(MAIN_DISP, user_input);
    printf_str(MAIN_DISP, DISP_X_COOR, DISP_Y_COOR, "请输入操作员卡密码", 1);
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
        // 下载充值配置参数
        ret = svc_get_download_dps_para(p16pos.devphyid);
        if(ret)
        {
            return -1;
        }
        ret = update_config_int("sys.cfgverno", cfgverno);
        if(ret)
        {
            disp_msg("更新系统版本号失败", 5);
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
		error_exit(2, "初始化屏幕对象失败");
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
        disp_msg("请安装PSAM卡！", 10);
        return;
    }
    // 检查操作员是否登录
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
            error_exit(3, "处理业务失败");
        }
        ++count;
    }
    if(count > 0)
    {
        disp_msg("处理异常流水完成", 5);
    }
    // 进入充值流程
    p16pos.last_trans_timeout = 0;
    do_main_loop();
    dc_reset(RFDEV, 0);
}
//}}}

