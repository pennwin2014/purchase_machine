#include "config.h"
#include "p16log.h"
#include "cardop.h"
#include "utils.h"
#include "menu.h"
#include "service.h"
#include "syspara.h"
#include "allowance.h"


//{{{ do_request_card function
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
        if (!is_beep)
            printf_str(MAIN_DISP, DISP_X_COOR + 30, DISP_Y_COOR + 50, "请放卡...", 1) ;
        else
            printf_str(MAIN_DISP, DISP_X_COOR + 30, DISP_Y_COOR + 50, "请确认放卡", 1);
        dis_mod_set(MAIN_DISP , INPUT_HZ_FONT , INPUT_ASC_FONT) ;
        // printf_str(MAIN_DISP, DISP_X_COOR + 20, DISP_Y_COOR + 80, "按<取消>退出" , 1);
        lcd_160_upd();

    }
    while (1)
    {
        ret = request_card_and_poweron(cardphyid, cardtype);
        if (0 == ret)
        {
            // memcpy(p16card.cardphyid, cardphyid , sizeof(p16card.cardphyid));
            // p16card.cardtype = cardtype;
            return 0;
        }
        tick2 = get_time_tick();
        if (tick != tick2)
        {
            tick = tick2;
            if (is_beep)
                beep(20);
        }
        uint8 keyval;
        ret = get_key(&keyval, TIM_DLY_MOD, 30);
        if (0 == ret)
        {
            if (F1_KEY == keyval)
            {
                return 1;
            }
        }
    }
    return -1;
}
//}}}
typedef struct
{
    char check_card_switch;
    uint8 cardphyid[17];
    int wait_key_switch;
    int wait_key;
    int beep_switch;
    int wait_card_switch;
} message_disp_t;

enum
{
    disp_keypress = 1, // 用户按键返回
    disp_noncard = 0,  // 未寻到卡
    disp_not_same_card = -1,  // 不是同一张卡
    disp_find_card = 2 // 寻到指定卡
};

/**
 * @brief 显示指定的信息，并检测卡片
 *
 * @param msg - 显示的信息
 * @param count - 信息条数
 * @param cardphyid - 检测的卡物理ID, 如果NULl 不判断是否是同一张卡
 *
 * @return 1 表示用户按键返回，0 表示用户移走卡片, -1 表示不是同一张卡
 */
static int do_disp_message_request_card(char msg[][64], int count,
                                        const message_disp_t* disp)
{
    int x, y, row;
    const int height = 40;
    CLEAR_SCREEN;
    x = DISP_X_COOR;
    y = DISP_Y_COOR;

    for (row = 0; row < count && row < 5; ++row)
    {
        printf_str(MAIN_DISP, x, y , (uint8*)msg[row], 1);
        y += height;
    }
    lcd_160_upd();
    int tick, tick2;
    tick = get_time_tick();
    while (1)
    {
        int ret;
        uint8 current_cardphyid[16];
        uint8 current_cardtype;
        memset(current_cardphyid, 0, sizeof current_cardphyid);
        ret = request_card_and_poweron(current_cardphyid, &current_cardtype);
        if (ret)
        {
            if (disp->wait_card_switch)
            {
                delay(10);
                continue;
            }
            return disp_noncard;
        }
        if (disp->check_card_switch)
        {
            if (memcmp(current_cardphyid, disp->cardphyid, 8) != 0)
            {
                return disp_not_same_card;
            }
            if (disp->wait_card_switch)
            {
                if (disp->wait_key_switch)
                {
                    uint8 key;
                    while (1)
                    {
                        ret = get_key(&key, BLOCK_MOD, 1);
                        if (ret == 0 && key == disp->wait_key)
                        {
                            return disp_keypress;
                        }
                    }
                }
                return disp_find_card;
            }
        }
        if (disp->wait_key_switch)
        {
            uint8 key;
            ret = get_key(&key, NOBLOCK_MOD, 20);
            if (ret == 0 && key == disp->wait_key)
            {
                return disp_keypress;
            }
        }
        tick2 = get_time_tick();
        if (tick != tick2)
        {
            tick = tick2;
            if (disp->beep_switch)
            {
                beep(20);
            }
        }
        delay(10);
    }
    return disp_noncard;
}
static void do_write_card(p16_transdtl_t* transdtl, allowance_info_t* info)
{
    int ret;
    char msg[5][64];
    int row;
    message_disp_t disp;
    memset(&disp, 0, sizeof disp);
    int get_flag = FLAG_TRANSACTION;
    while (1)
    {
        ret = init_4_load(&p16card);
        if (ret)
        {
            row = 0;
            strcpy(msg[row++], "卡片初始化失败");
            msg[row++][0] = '\0';
            strcpy(msg[row++], "请放好卡按[确认]");
            memset(&disp, 0, sizeof disp);
            disp.check_card_switch = 1;
            memcpy(disp.cardphyid, p16card.cardphyid, 8);
            disp.wait_key_switch = 1;
            disp.wait_key = ENT_KEY;
            disp.wait_card_switch = 1;
            ret = do_disp_message_request_card(msg, row, &disp);
            if (ret == disp_keypress)
            {
                continue;
            }
        }
        encode_hex(p16card.random_num, 4, transdtl->random);
        encode_hex(p16card.mac1, 4, transdtl->mac1);
        ret = svc_get_allowance(transdtl, info, get_flag);
        get_flag = FLAG_QUERY;
        if (ret)
        {
            if (ret == SVC_TIMEOUT)
            {
                // retry
                disp_msg("请求超时", 3);
                continue;
            }
            else
            {
                row = 0;
                sprintf(msg[row++], "领取补助失败: ");
                strncpy(msg[row++], transdtl->errmsg, sizeof(msg[0]) - 1);
                memset(&disp, 0, sizeof disp);
                do_disp_message_request_card(msg, row, &disp);
                break;
            }
        }
        else
        {
            decode_hex(transdtl->mac2, 8, p16card.mac2);
            SAFE_STR_CPY(p16card.hostdate, transdtl->hostdate);
            SAFE_STR_CPY(p16card.hosttime, transdtl->hosttime);

            /* disp_msg("等待写卡", 5); */
            ret = debit_4_load(&p16card);
            if (ret)
            {
                transdtl->transflag = TRANS_WRITE_CARD_FAILED;
                row = 0;
                sprintf(msg[row++], "写补助失败");
                msg[row++][0] = '\0';
                sprintf(msg[row++], "请放卡按[确认]键");
                memset(&disp, 0, sizeof disp);
                disp.wait_key_switch = 1;
                disp.wait_key = ENT_KEY;
                disp.check_card_switch = 1;
                memcpy(disp.cardphyid, p16card.cardphyid, 8);
                disp.wait_card_switch = 1;
                ret = do_disp_message_request_card(msg, row, &disp);
                if (ret == disp_keypress)
                {
                    // 寻卡成功，重试写卡
                    continue;
                }
                else
                {
                    disp_msg("补助写卡失败", 5);
                }
            }
            else
            {
                transdtl->transflag = TRANS_WRITE_CARD_OK;
                row = 0;
                sprintf(msg[row++], "领取补助成功");
                sprintf(msg[row++], "卡余额  : %.02f元",
                        (transdtl->cardbefbal + transdtl->dpsamt) / 100.0);
                sprintf(msg[row++], "补助金额: %.02f元", transdtl->dpsamt / 100.0);
                sprintf(msg[row++], "其它金额: %.02f元",
                        (transdtl->amount - transdtl->dpsamt) / 100.0);
                memset(&disp, 0, sizeof disp);
                do_disp_message_request_card(msg, row, &disp);
            }
            svc_deposit_confirm(transdtl, &p16pos.termseqno);
            if (transdtl->transflag == TRANS_WRITE_CARD_OK)
            {
                break;
            }
        }
    }
}
//{{{ do_write_allowance function
static void do_write_allowance(allowance_info_t* info)
{
    int ret;
    char msg[5][64];
    int row;
    p16_transdtl_t transdtl;
    memset(&transdtl, 0 , sizeof transdtl);
    char current_date[21];
    SAFE_GET_DATETIME("%Y%m%d", current_date);
    memcpy(p16card.termdate, current_date, 8);
    memcpy(p16card.termtime, current_date + 8, 6);
    // 记录流水
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
    transdtl.amount = info->total_get_amt;
    transdtl.dpsamt = p16card.dpsamt; // 实际充值金额
    transdtl.confirm = 0;
    transdtl.transflag = TRANS_INIT;
    transdtl.devseqno = p16pos.termseqno;
    transdtl.authcode = p16pos.authcode;
    transdtl.batchno = p16pos.batchno;
    transdtl.termid = p16pos.termid;

    message_disp_t disp;
    while (1)
    {
        do_write_card(&transdtl, info);
        // if write card failed
        if (transdtl.transflag !=  TRANS_WRITE_CARD_OK)
        {
            row = 0;
            sprintf(msg[row++], "写卡失败，请重刷卡！");
            memset(&disp, 0, sizeof disp);
            disp.check_card_switch = 1;
            memcpy(disp.cardphyid, p16card.cardphyid, 8);
            disp.beep_switch = 1;
            ret = do_disp_message_request_card(msg, row, &disp);
            if (ret == disp_find_card)
            {
                continue;
            }
            else
            {
                return;
            }
        }
        else if (transdtl.transflag == TRANS_WRITE_CARD_OK)
        {
            return;
        }
    }
}
//}}}
//{{{ do_allowance function
static void do_allowance()
{
    // read card information
    int ret;
    char emsg[257];
    memset(emsg, 0, sizeof emsg);
    ret = read_user_card(&p16card, emsg);
    if (ret)
    {
        LOG((LOG_ERROR, "读卡失败"));
        disp_msg(emsg, 1);
        return;
    }
    if (p16card.cardstatus != 0)
    {
        disp_msg("卡已锁", 3);
        return;
    }
    char current_date[21];
    uint8 hex_date[4];
    SAFE_GET_DATETIME("%Y%m%d", current_date);
    decode_hex(current_date, 8, hex_date);
    if (memcmp(hex_date, p16card.expiredate, 4) > 0)
    {
        disp_msg("卡已过期", 3);
        return;
    }

    // query allowance detail
    allowance_info_t info;
    memset(&info, 0, sizeof info);
    char msg[4][64];
    int row = 0;
    message_disp_t disp;
    memset(&disp, 0, sizeof disp);
    if (svc_query_allowance(&info) != SVC_OK)
    {
        disp_msg("查询补助失败", 5);
        return;
    }
    if (info.allowance_count == 0)
    {
        row = 0;
        msg[row++][0] = 0;
        msg[row++][0] = 0;
        strcpy(msg[row++], "暂无补助可领取");
        do_disp_message_request_card(msg, row, &disp);
    }
    else
    {
        sprintf(msg[row++], "您共有补助: %.02f元", info.total_amount / 100.0);
        sprintf(msg[row++], "可领取金额: %.02f元", info.total_get_amt / 100.0);
        sprintf(msg[row++], "卡余额    : %.02f元", p16card.cardbefbal / 100.0);
        sprintf(msg[row++], "请按绿色[确认]键领取");
        disp.check_card_switch = 1;
        memcpy(disp.cardphyid, p16card.cardphyid, 8);
        disp.wait_key_switch = 1;
        disp.wait_key = ENT_KEY;
        ret = do_disp_message_request_card(msg, row, &disp);
        if (ret == 1)
        {
            do_write_allowance(&info);
        }
        if (info.allowance_count > 0)
        {
            free(info.detail);
        }
    }
}
//}}}
//{{{ is_cancel function
/**
 * @brief 判断是否退出确认框
 *
 * @return 0 - 表示密码正确，其它表示错误
 */
static int is_cancel()
{
    const int wait_timeout = 15;
    uint8 key;
    char value[64];
    int ret, timeout, upd, value_len;
    uint8 input_value;
    uint32 tick1, tick2;
    uint16 x_coor, y_coor;

    char admin_pswd[64] = {0};
    ret = SAFE_GET_CONFIG("sys.password", admin_pswd);
    if (ret)
    {
        disp_msg("不能获取管理密码", 5);
        return -1;
    }

    tick1 = 0;
    timeout = wait_timeout;
    upd = 1;
    CLEAR_SCREEN;
    x_coor = DISP_X_COOR;
    y_coor = DISP_Y_COOR + 10;
    printf_str(MAIN_DISP, x_coor, y_coor, "请输入管理员密码", 1);
    y_coor += 40;
    memset(value, 0, sizeof value);
    while (1)
    {
        tick2 = get_time_tick();
        if (tick1 != tick2)
        {
            --timeout;
            if (timeout <= 0)
            {
                return -1;
            }
            tick1 = tick2;
        }

        if (upd)
        {
            upd = 0;
            CLEAR(MAIN_DISP, x_coor, y_coor, x_coor + 120, y_coor + INPUT_HEIGHT);
            box(MAIN_DISP, x_coor, y_coor, x_coor + 120, y_coor + INPUT_HEIGHT, WHITE_COLR);
            rectangle(MAIN_DISP, x_coor, y_coor, x_coor + 120, y_coor + INPUT_HEIGHT, 1);
            if (value_len > 0)
            {
                char pass_str[64];
                memset(pass_str, '*', value_len);
                pass_str[value_len] = 0;
                printf_str(MAIN_DISP, x_coor + 2, y_coor, pass_str, 1);
            }
            lcd_160_upd();
        }
        ret = get_key(&key, NOBLOCK_MOD, 1);
        if (ret == 0)
        {
            if (key == KEY_SHIFT)
            {
                if (value_len > 0)
                {
                    value_len--;
                    value[value_len] = 0;
                    upd = 1;
                }
            }
            else if (key == ENT_KEY)
            {
                if (value_len > 0)
                {
                    // check user password
                    if (strcmp(admin_pswd, value) == 0)
                    {
                        return 0;
                    }
                    else
                    {
                        disp_msg("管理员密码错误", 3);
                    }
                }
            }
            else if (is_number_key(key, &input_value))
            {
                if (value_len < 8)
                {
                    value[value_len++] = input_value + 0x30;
                    upd = 1;
                }
            }
        }

    }
}
//}}}

//{{{ main function
void allowance_menu()
{
    if (svc_app_login() != SVC_OK)
    {
        disp_msg("APP 签到错误", 10);
        return;
    }
    if (svc_allowance_machine_login() != SVC_OK)
    {
        disp_msg("补助机签到失败", 10);
        return;
    }
    disp_msg("签到成功", 4);
    int8 ret;
    while (1)
    {
        /*
        ret = get_key(&key, NOBLOCK_MOD , 1) ;
        if (ret == 0)
        {
            LOG((LOG_DEBUG, "Key value: %d", key));
            if (key == F1_KEY)
            {
                if (is_cancel())
                {
                    return;
                }
            }
        }
        */
        ret = do_request_card(0, p16card.cardphyid, &(p16card.cardtype));
        if (ret == 0)
        {
            do_allowance();
        }
        else if (ret > 0)
        {
            if (is_cancel())
            {
                continue;
            }
            return;
        }
        else
        {
            usleep(50 * 1000);
            continue;
        }
    }
}
//}}}

