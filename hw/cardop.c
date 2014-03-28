/******************************************************************************
* Copyright SupWisdom 2012 all right reserved
* Auth: Cheng Tang<cheng.tang@supwisdom.com>
******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>


#include "cardop.h"
#include "p16log.h"
#include "gnudes.h"
#include "utils.h"

static const uint8 card_aid[] = "\xDF\x03";
////////////////////////////////////////////////////////////////////////////////
void dec2hex(const unsigned char* uDecBuf, int iDecLen, char* sHexStr)
{  
	int i = 0;  
	int k = 0;  
	for(i = 0; i < iDecLen; i++)  
	{    
		k = uDecBuf[i];    
		sprintf(&sHexStr[2 * i], "%02X", k);  
	}
}

void hex2dec(const char* sHexStr, int iHexLen, unsigned char* uDecBuf, int* iDecBUfLen)
{  
	int i = 0;  
	unsigned long ul;  
	char sHexTmp[3];  
	int offset = 0;  
	int dlen = iHexLen / 2;  
	memset(sHexTmp, 0, sizeof(sHexTmp));  
	if(iHexLen % 2)  
	{    
		sHexTmp[0] = '0';    
		sHexTmp[1] = sHexStr[0];    
		ul = strtoul(sHexTmp, NULL, 16);    
		uDecBuf[0] = (unsigned char)ul;   
		offset++;  
	}  
	for(i = 0; i < dlen; i++)  
	{    
		memcpy(sHexTmp, &sHexStr[2 * i + offset], 2);   
		ul = strtoul(sHexTmp, NULL, 16);    
		uDecBuf[i + offset] = (unsigned char)ul; 
	}  
	*iDecBUfLen = i + offset;
}


static int32 get_int_bigend(uint8* data, uint8 len)
{
    uint8 bit_off[] = {0, 8, 16, 24};
    assert(len <= sizeof(bit_off));
    int32 t = 0;
    int i;
    for (i = 0; i < len; ++i)
    {
        t |= ((int32)data[i]) << bit_off[len - i - 1];
    }
    return t;
}
static int32 get_4byte_int(uint8* data)
{
    return get_int_bigend(data, 4);
}
static int32 get_3byte_int(uint8* data)
{
    return get_int_bigend(data, 3);
}
static int32 get_2byte_int(uint8* data)
{
    return get_int_bigend(data, 2);
}

static void do_set_int_bigend(uint32 value, uint8* data, uint8 size)
{
    assert(size > 0 && size < 5);
    int i, j;
    int bit_off[] = {24, 16, 8, 0};
    j = 4 - size;
    for (i = 0; i < size; ++i, ++j)
    {
        data[i] = (uint8)((value >> bit_off[j]) & 0xFF);
    }
}
static void set_4byte_int(uint32 value, uint8 data[4])
{
    do_set_int_bigend(value, data, 4);
}
static void set_2byte_int(uint32 value, uint8 data[2])
{
    do_set_int_bigend(value, data, 2);
}
////////////////////////////////////////////////////////////////////////////////
typedef struct
{
    uint8 slen;
    uint8 sbuffer[256];
    uint8 rlen;
    uint8 rbuffer[256];
    uint32 sw;
} cpu_apdu_t;

#define SAM_TIMEOUT 10 // 表示 10 * 10ms 

typedef int32(* apdu_command_func)(int fd, unsigned int, unsigned char*,
                                   unsigned int*, unsigned char*);

#define MAKESW(sw1,sw2) ((((uint32)sw1) << 8) | sw2)

/**
 * @brief - 执行adpu指令
 * @param cmd - 指令数据
 * @param apdu_func - 执行adpu函数
 * @return - 返回 0 表示执行成功，返回 1 表示APDU响应码<>9000 , 返回-1表示失败
 */
static int8 do_apdu(cpu_apdu_t* cmd, apdu_command_func apdu_func, int rf)
{
    int32 ret;
    uint32 recvlen, sendlen;
    uint8 sendbuffer[256];
    memcpy(sendbuffer, cmd->sbuffer, cmd->slen);
    sendlen = cmd->slen;
    while (1)
    {
        LOG( ( LOG_DEBUG, "执行CPU APDU 指令。。。" ) );
        ret = apdu_func(rf, sendlen, sendbuffer, &recvlen, cmd->rbuffer);
        if (ret)
        {
            LOG((LOG_ERROR, "执行APDU指令失败，返回ret=%d", abs(ret)));
            return -1;
        }

        uint8 sw1, sw2;
        sw1 = cmd->rbuffer[recvlen - 2];
        sw2 = cmd->rbuffer[recvlen - 1];
        cmd->sw = MAKESW(sw1, sw2);
        if (sw1 == 0x90 && sw2 == 0x00)
        {
            cmd->rlen = (uint8)recvlen;
            return 0;
        }
        else if (sw1 == 0x61)
        {
            if (sw2 > 0)
            {
                memcpy(sendbuffer, "\x00\xC0\x00\x00", 4);
                sendbuffer[4] = sw2;
                sendlen = 5;
            }
            else
            {
                cmd->rlen = (uint8)recvlen;
                return 0;
            }
        }
        else
        {
            return 1;
        }
    }
}

static int32 cpu_apdu_proxy(int fd, unsigned int slen, unsigned char* sbuf,
                            unsigned int* rlen, unsigned char* rbuf)
{
    int32 ret =  dc_pro_command(fd, slen, sbuf, rlen, rbuf, SAM_TIMEOUT);

    // if(!ret)
    // {
    //    char temp[512];
    //    memset(temp,0,sizeof temp);
    //    encode_hex(rbuf,*rlen,temp);
    //    LOG((LOG_DEBUG,"APDU: %s",temp));
    // }

    return ret;
}
static int32 sam_apdu_proxy(int fd, unsigned int slen, unsigned char* sbuf,
                            unsigned int* rlen, unsigned char* rbuf)
{
    int32 ret =  dc_cpuapdu(fd, slen, sbuf, rlen, rbuf);
    return ret;
}
static int8 cpu_apdu(cpu_apdu_t* cmd)
{
    return do_apdu(cmd, &cpu_apdu_proxy , RFDEV);
}
static int8 sam_apdu(cpu_apdu_t* cmd)
{
    return do_apdu(cmd, &sam_apdu_proxy , SAMDEV);
}
/**
 * @brief - 读取CPU卡binary file数据
 * @param fid - 文件FID
 * @param offset - 偏移
 * @param len - 读取长度
 * @param out_data - 返回读取的数据
 * @return - 返回 0 表示成功，其它表示失败
 */
static int8 cpu_read_file(uint8 fid, uint8 offset, uint8 len, uint8* out_data)
{
    cpu_apdu_t cmd;
    memset(&cmd, 0, sizeof cmd);
    memcpy(cmd.sbuffer, "\x00\xB0", 2);
    cmd.slen = 2;
    cmd.sbuffer[cmd.slen++] = 0x80 + fid;
    cmd.sbuffer[cmd.slen++] = offset;
    cmd.sbuffer[cmd.slen++] = len;
    int8 ret = cpu_apdu(&cmd);
    if (ret)
    {
        LOG((LOG_ERROR, "读取CPU卡文件失败，fid=%02X,sw=%04X", (uint32)fid, cmd.sw));
    }
    else
    {
        //LOG( ( LOG_DEBUG, "读取CPU卡文件成功" ) );
        memcpy(out_data, cmd.rbuffer, len);
    }
    return ret;
}
static int8 cpu_poweron()
{
    uint8 rlen, rbuf[256];
    if (dc_pro_reset(RFDEV, &rlen, rbuf))
    {
        LOG((LOG_DEBUG, "CPU卡上电错误!"));
        return -1;
    }
    return 0;
}
static int8 cpu_select_adf(const uint8* aid)
{
    cpu_apdu_t cmd;
    memset(&cmd, 0, sizeof cmd);
    memcpy(cmd.sbuffer, "\x00\xA4\x00\x00\x02", 5);
    cmd.slen = 5;
    memcpy(cmd.sbuffer + cmd.slen, aid, 2);
    cmd.slen += 2;
    cmd.sbuffer[cmd.slen++] = 0;
    int8 ret = cpu_apdu(&cmd);
    if (ret)
    {
        LOG((LOG_ERROR, "选择CPU卡目录失败,sw=%04X", cmd.sw));
    }
    return ret;
}

static int8 cpu_read_card(p16_card_context* cardctx, char emsg[256])
{

    int8 ret;
    cpu_apdu_t cmd;
    // memcpy(cmd.sbuffer, "\x00\x84\x00\x00\x04", 5);
    // cmd.slen = 5;
    // ret = cpu_apdu(&cmd);
    // if(ret)
    // {
    //   LOG((LOG_ERROR,"取随机数失败,sw=%04x", cmd.sw));
    //   return -1;
    // }

    ret = cpu_select_adf(card_aid);
    if (ret)
    {
        strcpy(emsg, "卡应用目录不存在");
        return -1;
    }
    LOG((LOG_DEBUG, "选择应用目录成功"));
    char temp[64];
    uint8 file15[56];
    ret = cpu_read_file(0x15, 0, 56, file15);
    if (ret)
    {
        strcpy(emsg, "无法读取卡基本信息");
        return -1;
    }
    // 交易卡号
    uint8 i = 0;
    while (i < 10 && file15[i] == 0) ++i;
    if (i == 10)
    {
        strcpy(emsg, "卡号不能为0");
        return -1;
    }
    memset(temp, 0, sizeof temp);
    fun_bcd_ascii(file15 + i, (uint8*)temp, 10 - i);
    cardctx->cardno = atoi(temp);
    // 卡状态
    cardctx->cardstatus = file15[20];
    // 卡版本号
    memcpy(cardctx->cardverno, file15 + 21, 7);
    // 卡收费类别
    cardctx->feetype = file15[34];
    // 有效期
    memcpy(cardctx->expiredate, file15 + 40, 4);
    // 卡消费次数
    memset(&cmd, 0, sizeof cmd);
    // 发起消费初始化指令
    memcpy(cmd.sbuffer, "\x80\x50\x01\x02\x0B\x01\x00\x00\x00\x00\x11\x22\x33\x44\x55\x66",
           16);
    cmd.slen = 16;
    ret = cpu_apdu(&cmd);
    if (ret)
    {
        strcpy(emsg, "读取卡消费次数失败");
        return -1;
    }
    cardctx->cardbefbal = get_4byte_int(cmd.rbuffer);
    cardctx->paycnt = get_4byte_int(cmd.rbuffer + 4);
	LOG((LOG_DEBUG, "卡余额；%d, %d", cardctx->cardbefbal, cardctx->paycnt));
    return 0;
}
static int8 cpu_read_purchase_card(p16_card_context* cardctx, char emsg[256])
{

    int8 ret;
    cpu_apdu_t cmd;
    // memcpy(cmd.sbuffer, "\x00\x84\x00\x00\x04", 5);
    // cmd.slen = 5;
    // ret = cpu_apdu(&cmd);
    // if(ret)
    // {
    //   LOG((LOG_ERROR,"取随机数失败,sw=%04x", cmd.sw));
    //   return -1;
    // }

    ret = cpu_select_adf(card_aid);
    if (ret)
    {
        strcpy(emsg, "卡应用目录不存在");
        return -1;
    }
    LOG((LOG_DEBUG, "选择应用目录成功"));
    char temp[64];
    uint8 file15[56];
    ret = cpu_read_file(0x15, 0, 56, file15);
    if (ret)
    {
        strcpy(emsg, "无法读取卡基本信息");
        return -1;
    }
    // 交易卡号
    uint8 i = 0;
    while (i < 10 && file15[i] == 0) ++i;
    if (i == 10)
    {
        strcpy(emsg, "卡号不能为0");
        return -1;
    }
    memset(temp, 0, sizeof temp);
    fun_bcd_ascii(file15 + i, (uint8*)temp, 10 - i);
    cardctx->cardno = atoi(temp);
    // 卡状态
    cardctx->cardstatus = file15[20];
    // 卡版本号
    memcpy(cardctx->cardverno, file15 + 21, 7);
    // 卡收费类别
    cardctx->feetype = file15[34];
    // 有效期
    memcpy(cardctx->expiredate, file15 + 40, 4);
    // 卡消费次数
    memset(&cmd, 0, sizeof cmd);
    // 发起消费初始化指令
    memcpy(cmd.sbuffer, "\x80\x50\x01\x02\x0B\x01\x00\x00\x00\x00\x11\x22\x33\x44\x55\x66",
           16);
    cmd.slen = 16;
    ret = cpu_apdu(&cmd);
    if (ret)
    {
        strcpy(emsg, "读取卡消费次数失败");
        return -1;
    }
    cardctx->cardbefbal = get_4byte_int(cmd.rbuffer);
    cardctx->paycnt = get_4byte_int(cmd.rbuffer + 4);
	//LOG((LOG_DEBUG, "卡余额；%d, %d", cardctx->cardbefbal, cardctx->paycnt));
    return 0;
}

static int8 cpu_psam_purchase_mac(p16_card_context* cardctx)
{
	int ret = -1;
	//步骤1:上电
    ret = dc_setcpupara(SAMDEV, p16pos.samport, 0, 0x11);
    if (ret)
    {
		LOG((LOG_DEBUG, "SAM卡上电失败1"));
        return -1;
    }

    uint8 rdata[256] = {0};
    uint8 rlen;
    ret = dc_cpureset(SAMDEV, &rlen, rdata);
    if (ret)
    {
		LOG((LOG_DEBUG, "SAM卡上电失败2"));
        return -1;
    }
    LOG((LOG_DEBUG, "SAM reset rlen=%d", (uint32)rlen));
    cpu_apdu_t cmd;
    //步骤2:选择目录
    memset( &cmd, 0, sizeof cmd );
    cmd.slen = 7;
    memcpy( cmd.sbuffer, "\x00\xa4\x00\x00\x02\xdf\x03", cmd.slen );
    ret = sam_apdu( &cmd );
    if( ret )
    {
      LOG( ( LOG_ERROR, "选择SAM卡目录失败,sw=%04X", cmd.sw ) );
      return -1;
    }
 	LOG((LOG_DEBUG, "psam消费初始化选目录成功"));
    //步骤3:psam的消费初始化
	memset(&cmd, 0, sizeof cmd);
	cmd.slen = 0;
	memcpy(cmd.sbuffer+cmd.slen,"\x80\x70\x00\x00\x1C",5);
	cmd.slen += 5;
	memcpy(cmd.sbuffer+cmd.slen,cardctx->random_num,4);//伪随机数
	cmd.slen += 4;
    uint8 hex_paycnt[2];
	memset(hex_paycnt,0,2);
	set_2byte_int(cardctx->paycnt,hex_paycnt);
    memcpy(cmd.sbuffer+cmd.slen,hex_paycnt,2);//用户卡交易序号，2个字节    
	cmd.slen += 2;
	uint8 hex_payamt[4];
	memset(hex_payamt,0,4);
	set_4byte_int(cardctx->payamt,hex_payamt);
    memcpy(cmd.sbuffer+cmd.slen,hex_payamt,4);//交易金额，4个字节
	cmd.slen += 4;
	cmd.sbuffer[cmd.slen] = 6;
	cmd.slen += 1;
	//交易日期和时间  	
    char current_date[64];
	memset(current_date,0,sizeof current_date);
	SAFE_GET_DATETIME("%Y%m%d%H%M%S", current_date);
    memcpy(cardctx->termdate, current_date, 8);
    memcpy(cardctx->termtime, current_date + 8, 6);
	uint8 hex_date[4];
	uint8 hex_time[3];
	decode_hex(cardctx->termdate,8,hex_date);
	decode_hex(cardctx->termtime,6,hex_time);
	
	memcpy(cmd.sbuffer+cmd.slen,hex_date,4);//交易日期，4个字节
	cmd.slen += 4;
	memcpy(cmd.sbuffer+cmd.slen,hex_time,3);//交易时间，3个字节
	cmd.slen += 3;
	cmd.sbuffer[cmd.slen] = cardctx->key_verno;//消费密钥版本号，1个字节  
	cmd.slen += 1;
	cmd.sbuffer[cmd.slen] = cardctx->alg_ver;//消费算法标志，1个字节    
	cmd.slen += 1;
    //用户卡应用序列号    
	uint8 paste_phyid_uint8[4];
	memset(paste_phyid_uint8,0,4);
	decode_hex("80000000",8,paste_phyid_uint8);
	int i = 0;
	uint8 send_phyid_uint8[8];
	for(i=0;i<8;i++)
	{
		if(i<4)
			send_phyid_uint8[i] = cardctx->cardphyid[i];
		else
			send_phyid_uint8[i] = paste_phyid_uint8[i-4];
	}
	memcpy(cmd.sbuffer+cmd.slen,send_phyid_uint8,8);//// 卡序列号，8个字节    
	cmd.slen += 8;
	LOG((LOG_DEBUG, "拼出来的slen=%d",(uint32)cmd.slen));
	
	//"08"
	p16dump_hex(LOG_DEBUG, cmd.sbuffer, cmd.slen);
	
	ret = sam_apdu(&cmd);
	if(ret)
	{     
		LOG((LOG_DEBUG, "向SAM发初始化指令失败,%04X", cmd.sw));
		return ret;
	}
	LOG((LOG_DEBUG, "psam发初始化指令OK"));
	//取后续数据 
	memcpy(cardctx->sam_seqno,cmd.rbuffer,sizeof cardctx->sam_seqno);
	memcpy(cardctx->mac1,cmd.rbuffer+4,sizeof cardctx->mac1);
	//打印出来看一下
	uint32 sam_seq_no = 0;
	uint32 recv_mac1 = 0;
	sam_seq_no = get_4byte_int(cardctx->sam_seqno);
	recv_mac1 = get_4byte_int(cardctx->mac1);
	LOG((LOG_DEBUG, "sam_seqno=%d, mac1=%08X", sam_seq_no, recv_mac1));
	
	return 0;
}

static int8 cpu_debit_4_purchase(p16_card_context* cardctx)
{
	cpu_apdu_t cmd;
	int ret;
	//2 debit_4_purchase
	memset(&cmd, 0, sizeof cmd);
	cmd.slen = 0;
	memcpy(cmd.sbuffer+cmd.slen,"\x80\x54\x01\x00\x0F",5);
	cmd.slen += 5;
    //终端交易序号，4字节    
	memcpy(cmd.sbuffer+cmd.slen,cardctx->sam_seqno,4);
	cmd.slen += 4;
	//交易日期，4字节    
	uint8 hex_date[4];
	decode_hex(cardctx->termdate,8,hex_date);
	memcpy(cmd.sbuffer+cmd.slen,hex_date,4);
	cmd.slen += 4;
	//交易时间，3字节   
	uint8 hex_time[3];
	decode_hex(cardctx->termtime,6,hex_time);
	memcpy(cmd.sbuffer+cmd.slen,hex_time,3);
	cmd.slen += 3;
	//mac1，4字节
	memcpy(cmd.sbuffer+cmd.slen,cardctx->mac1,4);
	cmd.slen += 4;
	//08
	//memcpy(cmd.sbuffer+cmd.slen,"\x08",1);
	//cmd.slen += 1;
	p16dump_hex(LOG_DEBUG, cmd.sbuffer, cmd.slen);
	ret = cpu_apdu(&cmd);
    if (ret)
    {
        LOG((LOG_DEBUG, "消费确认失败，sw=%04X", cmd.sw));
        return -1;
    }
	LOG((LOG_DEBUG, "消费确认成功，sw=%04X", cmd.sw));
    memcpy(cardctx->tac,cmd.rbuffer,sizeof cardctx->tac);
	memcpy(cardctx->mac2,cmd.rbuffer+4,sizeof cardctx->mac2);
	return 0;
}

static int8 cpu_init_4_purchase(p16_card_context* cardctx)
{
	int ret;
	cpu_apdu_t cmd;
	//2 init for purchase
	memset(&cmd, 0, sizeof cmd);
	cmd.slen = 6;
	memcpy(cmd.sbuffer, "\x80\x50\x01\x02\x0B\x01", cmd.slen);
	//"\x00\x00\x00\x01"
	set_4byte_int(cardctx->payamt, &cmd.sbuffer[cmd.slen]);
	cmd.slen += 4;
	//"\x00\x00\x00\x00\x00\x01"
	memcpy(cmd.sbuffer + cmd.slen, p16pos.termno, 6);
	cmd.slen += 6;
	LOG((LOG_DEBUG,"消费初始化===================="));
	p16dump_hex(LOG_DEBUG,cmd.sbuffer,cmd.slen);
	//这里末尾的\x0F不发
	ret = cpu_apdu(&cmd);
	if (ret)
	{
		LOG((LOG_ERROR, "卡片消费初始化失败,sw=%04X", cmd.sw));
		return -1;
	}
	p16dump_hex(LOG_DEBUG,cmd.rbuffer,cmd.rlen);
	uint8 f = 0;
	cardctx->cardbefbal = get_4byte_int(cmd.rbuffer + f);
	f += 4;
	cardctx->paycnt = (uint16)get_2byte_int(cmd.rbuffer + f);
	f += 2;
	memcpy(cardctx->card_limit_amt,cmd.rbuffer + f,3);
	f += 3;
	cardctx->key_verno = cmd.rbuffer[f];
	f += 1;
	cardctx->alg_ver = cmd.rbuffer[f];
	f += 1;
	memcpy(cardctx->random_num, &cmd.rbuffer[f], 4);
	LOG((LOG_DEBUG, "卡余额 %u , 消费次数 %d ,",cardctx->cardbefbal, cardctx->paycnt));
	return 0;

}

static int8 cpu_init_4_load(p16_card_context* cardctx)
{
    cpu_apdu_t cmd;
    memset(&cmd, 0, sizeof cmd);
    //1 verify pin
    cmd.slen = 4;
    memcpy(cmd.sbuffer, "\x00\x20\x00\x00", cmd.slen);
    int32 pinlen = strlen(CPU_DEFAULT_PIN);
    cmd.sbuffer[cmd.slen++] = pinlen >> 1;
    fun_ascii_bcd(CPU_DEFAULT_PIN, cmd.sbuffer + cmd.slen, pinlen);
    cmd.slen += pinlen >> 1;
    int8 ret;
    ret = cpu_apdu(&cmd);
    if (ret)
    {
        LOG((LOG_ERROR, "卡片verify pin失败,sw=%04X", cmd.sw));
        return -1;
    }
    //2 init for load
    memset(&cmd, 0, sizeof cmd);
    cmd.slen = 6;
    memcpy(cmd.sbuffer, "\x80\x50\x00\x02\x0B\x01", cmd.slen);
    set_4byte_int(cardctx->dpsamt, &cmd.sbuffer[cmd.slen]);
    cmd.slen += 4;
    memcpy(cmd.sbuffer + cmd.slen, p16pos.termno, 6);
    cmd.slen += 6;
    //LOG((LOG_DEBUG,"充值初始化"));
    //p16dump_hex(LOG_DEBUG,cmd.sbuffer,cmd.slen);
    ret = cpu_apdu(&cmd);
    if (ret)
    {
        LOG((LOG_ERROR, "卡片充值初始化失败,sw=%04X", cmd.sw));
        return -1;
    }
    //p16dump_hex(LOG_DEBUG,cmd.rbuffer,cmd.rlen);
    uint8 f = 4;
    cardctx->cardbefbal = get_4byte_int(cmd.rbuffer);
    cardctx->dpscnt = (uint16)get_2byte_int(cmd.rbuffer + f);
    f += 2;
    // 密钥版本 + 密钥索引
    f += 2;
    memcpy(cardctx->random_num, cmd.rbuffer + f, 4);
    f += 4;
    memcpy(cardctx->mac1, cmd.rbuffer + f, 4);
    LOG((LOG_DEBUG, "卡余额 %d , 充值次数 %d", cardctx->cardbefbal, cardctx->dpscnt));
    return 0;
}
static int8 cpu_debit_4_load(p16_card_context* cardctx)
{
    cpu_apdu_t cmd;
    memset(&cmd, 0, sizeof cmd);

    cmd.slen = 5;
    memcpy(cmd.sbuffer, "\x80\x52\x00\x00\x0B", cmd.slen);
    decode_hex(cardctx->hostdate, 8, cmd.sbuffer + cmd.slen);
    cmd.slen += 4;
    decode_hex(cardctx->hosttime, 6, cmd.sbuffer + cmd.slen);
    cmd.slen += 3;
    memcpy(cmd.sbuffer + cmd.slen, cardctx->mac2, 4);
    cmd.slen += 4;
    cmd.sbuffer[cmd.slen++] = 0x04;

    int ret;
    //p16dump_hex(LOG_DEBUG,cmd.sbuffer,cmd.slen);
    ret = cpu_apdu(&cmd);
    if (ret)
    {
        if (cmd.sw == 0)
            return -1;
        LOG((LOG_ERROR, "debit 4 load ,sw=%04X", cmd.sw));
        return 1;
    }
    memcpy(cardctx->tac, cmd.rbuffer, 4);
    return 0;
}

static int8 cpu_get_purchase_card_prove(p16_card_context* cardctx)
{
	return 0;
}

static int8 cpu_get_card_prove(p16_card_context* cardctx)
{
    int ret;
    ret = cpu_select_adf(card_aid);
    if (ret)
    {
        return -1;
    }

    cpu_apdu_t cmd;
    memset(&cmd, 0, sizeof cmd);
    cmd.slen = 5;
    memcpy(cmd.sbuffer, "\x80\x5A\x00\x02\x02", cmd.slen);
    // fix by TC : 取交易认证应该是交易后的充值次数
    set_2byte_int(cardctx->dpscnt + 1, cmd.sbuffer + cmd.slen);
    cmd.slen += 2;
    cmd.sbuffer[cmd.slen++] = 0x08;

    ret = cpu_apdu(&cmd);
    if (ret)
    {
        LOG((LOG_ERROR, "取交易认证失败，sw=%04X", cmd.sw));
        if (cmd.sw == 0x9406)
            return 1;
        else
            return -1;
    }
    memcpy(cardctx->tac, cmd.rbuffer + 4, 4);
    return 0;
}
////////////////////////////////////////////////////////////////////////////////
static uint8 m1_card_keya[6];
static int m1_last_login_sect = 0xFF;
static const int purse_sectno = 4;
static void do_calc_card_keya(const uint8* cardphyid, uint8* keya)
{
    uint8 plain[8], cipher[16];
    uint16 i, pos;
    for (i = 0; i < 4; ++i)
    {
        plain[i] = cardphyid[i];
        plain[i + 4] = ~cardphyid[i];
    }
    des_context des_ctx;
    memset(&des_ctx, 0, sizeof des_ctx);
    // p16dump_hex(LOG_DEBUG, p16pos.device_key, 8);
    // p16dump_hex(LOG_DEBUG, plain, 8);
    des_set_key(&des_ctx, p16pos.device_key);
    des_encrypt(&des_ctx, plain, cipher);
    // p16dump_hex(LOG_DEBUG, cipher, 8);
    for (i = 0, pos = 0; i < 8; ++i)
    {
        pos += cipher[i];
    }
    memcpy(cipher + 8, cipher, 8);
    memcpy(keya, cipher + (pos % 7), 6);
    // p16dump_hex(LOG_DEBUG, keya, 6);
}
static int m1_login_card(p16_card_context* card, int sectno)
{
    if (m1_last_login_sect == sectno)
        return 0;
    if (dc_authentication_pass(RFDEV, 0, (uint8)sectno, m1_card_keya))
    {
        LOG((LOG_ERROR, "login sectno[%d] error", sectno));
        return -1;
    }
    m1_last_login_sect = sectno;
    return 0;
}
static int m1_read_block(p16_card_context* card, int sectno, int blockno, uint8* block)
{
    if (m1_login_card(card, sectno))
        return -1;
    blockno += (sectno << 2);
    if (dc_read(RFDEV, blockno, block))
        return -1;
    return 0;
}
static int m1_write_block(p16_card_context* card, int sectno, int blockno,
                          const uint8* block)
{
    if (m1_login_card(card, sectno))
        return -1;
    blockno += (sectno << 2);
    if (dc_write(RFDEV, blockno, (uint8*)block))
        return -1;
    return 0;
}
static uint8 m1_calc_crc(const uint8* data, uint8 len)
{
    uint8 result, i;
    for (i = 0, result = 0; i < len; ++i)
    {
        result += data[i];
    }
    return result;
}
static int m1_check_block_crc(const uint8* block_data)
{
    return (block_data[15] == m1_calc_crc(block_data, 15));
}

static int8 m1_read_card(p16_card_context* cardctx, char emsg[256])
{
    uint8 buffer[16], offset;
    int sectno = 1;
    int blockno = 0;
    if (m1_read_block(cardctx, sectno, blockno, buffer))
    {
        strcpy(emsg, "读取卡 1 扇区失败");
        LOG((LOG_ERROR, emsg));
        return -1;
    }
    cardctx->cardno = get_3byte_int(buffer);
    offset = 3;
    cardctx->feetype = buffer[offset++];
    cardctx->expiredate[0] = 0x20;
    // memcpy(cardctx->expiredate + 1, buffer + offset, 3);
    hex_to_bcd(buffer + offset, 3, cardctx->expiredate + 1);
    offset += 4;

    sectno = 3;
    if (m1_read_block(cardctx, sectno, 2, buffer))
    {
        strcpy(emsg, "读取卡 3 扇区失败");
        LOG((LOG_ERROR, emsg));
        return -1;
    }
    cardctx->cardstatus = (buffer[0] != 0) ? 0 : 3;

    sectno = purse_sectno;
    blockno = 0;
    if (m1_read_block(cardctx, sectno, blockno, buffer))
    {
        strcpy(emsg, "读取卡 4 扇区失败");
        LOG((LOG_ERROR, emsg));
        return -1;
    }
    if (!m1_check_block_crc(buffer))
    {
        if (m1_read_block(cardctx, sectno, blockno + 2, buffer))
        {
            strcpy(emsg, "读取卡 4 扇区失败");
            LOG((LOG_ERROR, emsg));
            return -1;
        }
        if (!m1_check_block_crc(buffer))
        {
            strcpy(emsg, "卡钱包验证错误");
            LOG((LOG_ERROR, emsg));
            return -1;
        }
    }
    offset = 0;
    cardctx->cardbefbal = buffer[offset++];
    cardctx->cardbefbal += ((uint32)buffer[offset++]) << 8;
    cardctx->cardbefbal += ((uint32)buffer[offset++]) << 16;
    cardctx->dpscnt = (uint16)get_2byte_int(buffer + offset);
    cardctx->paycnt = cardctx->dpscnt;
    memcpy(cardctx->m1_card_buffer, buffer, 16);
    return 0;
}
static void m1_calc_sk(p16_card_context* cardctx, uint8 sk[8])
{
    uint8 sk_data[64], offset, factor[8], cardkey[8], sk_enc_data[64];
    uint8 i;
    memcpy(sk_data, cardctx->random_num, 4);
    offset = 4;
    set_2byte_int(cardctx->dpscnt, sk_data + offset);
    offset += 2;
    memcpy(sk_data + offset, "\x80\x00", 2);
    offset += 2;
    // p16dump_hex(LOG_DEBUG, sk_data, offset);

    memcpy(factor, cardctx->cardphyid, 4);
    memcpy(factor + 4, "\x80\x00\x00\x00", 4);
    des_context ctx;
    memset(&ctx, 0, sizeof ctx);
    des_set_key(&ctx, p16pos.device_key);
    des_encrypt(&ctx, factor, cardkey);

    for (i = 0; i < offset; i += 8)
    {
        memset(&ctx, 0 , sizeof ctx);
        des_set_key(&ctx, cardkey);
        des_encrypt(&ctx, sk_data + i, sk_enc_data + i);
    }
    memcpy(sk, sk_enc_data, 8);
}
static void pboc_calc_des_mac(const uint8 key[8], const uint8 init_data[8],
                              const uint8* data, uint32 datalen, uint8 mac[4])
{
    uint32 offset, i, j;
    uint8 enc_data[8], output[8];
    des_context ctx;
    const uint8 padding[] = "\x80\x00\x00\x00\x00\x00\x00\x00";
    memcpy(enc_data, init_data, 8);
    for (offset = 0; offset < datalen; offset += 8)
    {
        for (i = 0; i < 8 && i + offset < datalen; ++i)
        {
            enc_data[i] ^= data[offset + i];
        }
        for (j = i; j < 8; ++j)
        {
            enc_data[j] ^= padding[j - i];
        }
        memset(&ctx, 0, sizeof ctx);
        des_set_key(&ctx, (uint8*)key);
        des_encrypt(&ctx, enc_data, output);
        memcpy(enc_data, output, 8);
    }
    memcpy(mac, enc_data, 4);
}


static int8 m1_init_4_load(p16_card_context* cardctx)
{
    uint8 mac_data[64], sk[8], init_data[8];
    uint8 offset;
    set_4byte_int(cardctx->cardbefbal, mac_data);
    offset = 4;

    set_4byte_int(cardctx->dpsamt, mac_data + offset);
    offset += 4;

    mac_data[offset++] = 0x02;

    memcpy(mac_data + offset,  p16pos.termno, 6);
    offset += 6;
    mac_data[offset++] = 0x80;

    srand((unsigned int)time(NULL));
    int value = rand();
    set_4byte_int(value, cardctx->random_num);

    m1_calc_sk(cardctx, sk);
    // p16dump_hex(LOG_DEBUG, sk, 8);
    // p16dump_hex(LOG_DEBUG, mac_data, offset);
    memset(init_data, 0, sizeof init_data);
    pboc_calc_des_mac(sk, init_data, mac_data, offset, cardctx->mac1);
    return 0;
}

static int8 m1_debit_4_load(p16_card_context* cardctx)
{
    uint8 mac_data[64], offset;
    uint8 sk[8], mac2[4], buffer[16];
    uint32 cardbefbal, cardaftbal;
    uint16 dpscnt;
    set_4byte_int(cardctx->dpsamt, mac_data);
    offset = 4;
    mac_data[offset++] = 0x02;

    memcpy(mac_data + offset, p16pos.termno, 6);
    offset += 6;

    // memcpy(mac_data + offset, cardctx->hostdate, 4);
    decode_hex(cardctx->hostdate, 8, mac_data + offset);
    offset += 4;

    // memcpy(mac_data + offset, cardctx->hosttime, 3);
    decode_hex(cardctx->hosttime, 6, mac_data + offset);
    offset += 3;

    memcpy(mac_data + offset, "\x80\x00\x00\x00\x00\x00", 6);
    offset += 6;

    // p16dump_hex(LOG_DEBUG, mac_data, offset);
    m1_calc_sk(cardctx, sk);
    memset(buffer, 0, sizeof buffer);
    pboc_calc_des_mac(sk, buffer, mac_data, offset, mac2);
    if (memcmp(mac2, cardctx->mac2, 4) != 0)
    {
        LOG((LOG_DEBUG, "验证MAC2错误"));
        return -1;
    }
    memcpy(buffer, cardctx->m1_card_buffer, 16);
    // p16dump_hex(LOG_DEBUG, buffer, 16);
    offset = 0;
    cardbefbal = buffer[offset++];
    cardbefbal += ((uint32)buffer[offset++]) << 8;
    cardbefbal += ((uint32)buffer[offset++]) << 16;
    if (cardctx->cardbefbal != cardbefbal)
    {
        LOG((LOG_ERROR, "卡余额不符"));
        return -1;
    }
    dpscnt = (uint16)get_2byte_int(buffer + offset);
    if (dpscnt != cardctx->dpscnt)
    {
        LOG((LOG_ERROR, "卡充值次数不符"));
        return -1;
    }
    cardaftbal = cardbefbal + cardctx->dpsamt;
    // LOG((LOG_DEBUG, "balance : %d", cardaftbal));
    ++dpscnt;
    offset = 0;
    buffer[offset++] = (uint8)(cardaftbal & 0xFF);
    buffer[offset++] = (uint8)((cardaftbal >> 8) & 0xFF);
    buffer[offset++] = (uint8)((cardaftbal >> 16) & 0xFF);
    // p16dump_hex(LOG_DEBUG, buffer, 16);
    set_2byte_int(dpscnt, buffer + offset);
    offset += 2;
    // p16dump_hex(LOG_DEBUG, buffer, 16);
    uint8 date_temp[4];
    // LOG((LOG_DEBUG,"hostdatetime: %s : %s", cardctx->hostdate, cardctx->hosttime));
    decode_hex(cardctx->hostdate, 8, date_temp);
    // p16dump_hex(LOG_DEBUG, date_temp, 4);
    bcd_to_hex(date_temp + 1, 3, buffer + offset);
    offset += 3;
    decode_hex(cardctx->hosttime, 6, date_temp);
    bcd_to_hex(date_temp, 2, buffer + offset);
    offset += 2;
    buffer[15] = m1_calc_crc(buffer, 15);
    // p16dump_hex(LOG_DEBUG, buffer, 16);
    if (m1_write_block(cardctx, purse_sectno, 2, cardctx->m1_card_buffer))
    {
        LOG((LOG_ERROR, "更新钱包备份扇区失败"));
        return -1;
    }
    if (m1_write_block(cardctx, purse_sectno, 0, buffer))
    {
        LOG((LOG_ERROR, "写钱包失败"));
        return -1;
    }
    memcpy(cardctx->tac, "\x55\xaa\x55\xaa", 4);
    return 0;
}
static int8 m1_get_card_prove(p16_card_context* cardctx)
{
    uint8 buffer[16];
    uint16 dpscnt;
    if (m1_read_block(cardctx, purse_sectno, 0, buffer))
    {
        LOG((LOG_ERROR, "读钱包失败"));
        return -1;
    }
    if (!m1_check_block_crc(buffer))
    {
        if (m1_read_block(cardctx, purse_sectno, 2, buffer))
        {
            LOG((LOG_ERROR, "读钱包失败"));
            return -1;
        }
        if (!m1_check_block_crc(buffer))
        {
            LOG((LOG_ERROR, "钱包格式损坏"));
            return -1;
        }
    }
    dpscnt = (uint16)get_2byte_int(buffer + 3);
    if (dpscnt != cardctx->dpscnt + 1)
    {
        LOG((LOG_ERROR, "次数不符"));
        return -1;
    }
    memcpy(cardctx->tac, "\x55\xaa\x55\xaa", 4);
    return 0;
}
////////////////////////////////////////////////////////////////////////////////

/**
 * @brief - 寻卡函数
 * @param cardphyid - 返回卡片物理ID
 * @param cardtype - 返回卡片类型
 * @return - 返回 0 表示成功，其它表示未寻到卡
 */
int8 request_card(uint8 cardphyid[], uint8* cardtype)
{
    dc_reset(RFDEV, 0);
    usleep(30 * 1000);
    dc_reset(RFDEV, 1);
    int32 ret;
    uint8 tag_type;
    ret = dc_request(RFDEV, 1, &tag_type);
    if (ret)
        return -1;

    uint32 uid;
    ret = dc_anticoll(RFDEV, 0, &uid);
    if (ret)
        return -1;

    uint8 size;
    ret = dc_select(RFDEV, uid, &size);
    if (ret)
        return -1;

    uint8* p;
    // LOG((LOG_DEBUG, "request card cardtype=%d,size=%d", (uint32)tag_type, (uint32)size));
    if (tag_type == 2)
        *cardtype = CT_M1;
    else if (tag_type == 4)
    {
        if (size == 40)
            *cardtype = CT_CPU;
        else // if(size == 8 || size == 32)
            *cardtype = CT_M1;
    }
    switch (*cardtype)
    {
    case CT_M1:
        memcpy(cardphyid, (uint8*)&uid, 4);
        // p16dump_hex(LOG_DEBUG, cardphyid, 4);
        do_calc_card_keya(cardphyid, m1_card_keya);
        m1_last_login_sect = 0xFF;
        break;
    case CT_CPU:
        p = (uint8*)&uid;
        cardphyid[0] = p[3];
        cardphyid[1] = p[2];
        cardphyid[2] = p[1];
        cardphyid[3] = p[0];
        break;
    default:
        return -1;
    }
    return 0;
}
/**
 * @brief - 读卡信息
 * @param cardctx - 返回卡信息参数
 * @param emsg - 返回错误信息
 * @return - 返回 0 表示读卡成功，其它表示读卡失败
 */
int8 read_user_card(p16_card_context* cardctx , char emsg[256])
{
    switch (cardctx->cardtype)
    {
    case CT_M1:
        return m1_read_card(cardctx, emsg);
    case CT_CPU:
        return cpu_read_card(cardctx, emsg);
    default:
        strcpy(emsg, "不支持卡类型");
        return -1;
    }
    return 0;
}
/**
 * @brief - 读卡信息
 * @param cardctx - 返回卡信息参数
 * @param emsg - 返回错误信息
 * @return - 返回 0 表示读卡成功，其它表示读卡失败
 */
int8 read_user_purchase_card(p16_card_context* cardctx , char emsg[256])
{
    switch (cardctx->cardtype)
    {
    case CT_M1:
        return -1;//m1_read_card(cardctx, emsg);
    case CT_CPU:
        return cpu_read_purchase_card(cardctx, emsg);
    default:
        strcpy(emsg, "不支持卡类型");
        return -1;
    }
    return 0;
}

int8 request_card_and_poweron(uint8 cardphyid[], uint8* cardtype)
{
    int ret = request_card(cardphyid, cardtype);
    if (!ret)
    {
        if (*cardtype == CT_CPU)
        {
            LOG((LOG_DEBUG, "cpu card poweron"));
            if (cpu_poweron())
                return -1;
        }
        return 0;
    }
    else
    {
        return ret;
    }
}
/**
 * @brief - 充值初始化
 * @param cardctx - 卡上下文属性
 * @return - 返回0 表示初始化成功，其它表示失败
 */
int8 init_4_load(p16_card_context* cardctx)
{
    switch (cardctx->cardtype)
    {
    case CT_M1:
        return m1_init_4_load(cardctx);
    case CT_CPU:
        return cpu_init_4_load(cardctx);
    default:
        LOG((LOG_ERROR, "不支持卡类型"));
        return -1;
    }
    return 0;
}
/**
 * @brief - 充值写卡确认
 * @param cardctx - 卡上下文
 * @return - 返回0 表示成功，-1 表示写卡失败，1 表示中途拔卡
 */
int8 debit_4_load(p16_card_context* cardctx)
{
    switch (cardctx->cardtype)
    {
    case CT_M1:
        return m1_debit_4_load(cardctx);
    case CT_CPU:
        return cpu_debit_4_load(cardctx);
    default:
        LOG((LOG_ERROR, "不支持卡类型"));
        return -1;
    }
    return 0;
}

int8 psam_purchase_mac(p16_card_context* cardctx)
{
	switch (cardctx->cardtype)
    {
    case CT_M1:
        return -1;
    case CT_CPU:
        return cpu_psam_purchase_mac(cardctx);
    default:
        LOG((LOG_ERROR, "不支持卡类型"));
        return -1;
    }
    return 0;
}	

int8 init_4_purchase(p16_card_context* cardctx)
{
	switch (cardctx->cardtype)
    {
    case CT_M1:
        return -1;
    case CT_CPU:
        return cpu_init_4_purchase(cardctx);
    default:
        LOG((LOG_ERROR, "不支持卡类型"));
        return -1;
    }
    return 0;
}
int8 debit_4_purchase(p16_card_context* cardctx)
{
	switch (cardctx->cardtype)
    {
    case CT_M1:
        return -1;
    case CT_CPU:
        return cpu_debit_4_purchase(cardctx);
    default:
        LOG((LOG_ERROR, "不支持卡类型"));
        return -1;
    }
    return 0;
}

/**
 * @brief - 读卡充值认证
 * @param cardctx - 卡上下文
 * @return - 返回 0 表示充值成功，1 表示充值不成功，-1 表示读卡失败
 */
int8 get_card_prove(p16_card_context* cardctx)
{
    switch (cardctx->cardtype)
    {
    case CT_M1:
        return m1_get_card_prove(cardctx);
    case CT_CPU:
        return cpu_get_card_prove(cardctx);
    default:
        LOG((LOG_ERROR, "不支持卡类型"));
        return -1;
    }
    return 0;
}
/**
 * @brief - 读卡消费认证
 * @param cardctx - 卡上下文
 * @return - 返回 0 表示消费成功，1 表示消费不成功，-1 表示读卡失败
 */
int8 get_purchase_card_prove(p16_card_context* cardctx)
{
    switch (cardctx->cardtype)
    {
    case CT_M1:
        return -1;
    case CT_CPU:
        return cpu_get_purchase_card_prove(cardctx);
    default:
        LOG((LOG_ERROR, "不支持卡类型"));
        return -1;
    }
    return 0;
}

int8 sam_read_termno(uint8* termno)
{
   p16pos.samport = 0x0d; // SAM1
   // p16pos.samport = 0x0e;
    int ret;

    ret = dc_setcpu(SAMDEV, p16pos.samport);
    if (ret)
    {
        LOG((LOG_ERROR, "设置卡座失败"));
        return -1;
    }
    ret = dc_setcpupara(SAMDEV, p16pos.samport, 0, 0x11);
    if (ret)
    {
        LOG((LOG_ERROR, "SAM卡上电失败1"));
        return -1;
    }

    uint8 rdata[256] = {0};
    uint8 rlen;
    ret = dc_cpureset(SAMDEV, &rlen, rdata);
    if (ret)
    {
        LOG((LOG_ERROR, "SAM卡上电失败2"));
        return -1;
    }
    LOG((LOG_DEBUG, "SAM reset rlen=%d", (uint32)rlen));
    cpu_apdu_t cmd;
    /*
    memset( &cmd, 0, sizeof cmd );
    cmd.slen = 7;
    memcpy( cmd.sbuffer, "\x00\xA4\x00\x00\x02\x3F\x00", cmd.slen );
    ret = sam_apdu( &cmd );
    if( ret )
    {
      LOG( ( LOG_ERROR, "选择SAM卡目录失败,sw=%04X", cmd.sw ) );
      return -1;
    }
    */

    memset(&cmd, 0, sizeof cmd);
    memcpy(cmd.sbuffer, "\x00\xB0", 2);
    cmd.slen = 2;
    cmd.sbuffer[cmd.slen++] = 0x80 + 0x16;
    cmd.sbuffer[cmd.slen++] = 0;
    cmd.sbuffer[cmd.slen++] = 6;
    ret = sam_apdu(&cmd);
    if (ret)
    {
        LOG((LOG_ERROR, "读取SAM卡文件失败，fid=0x16,sw=%04X", cmd.sw));
        return -1;
    }
    else
    {
        memcpy(termno, cmd.rbuffer, 6);
        return 0;
    }

}

