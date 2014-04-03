/******************************************************************************
* Copyright SupWisdom 2012 all right reserved
* Module: P16 utils function
* File: utils.c
* Auth: Cheng Tang<cheng.tang@supwisdom.com>
******************************************************************************/

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#define __USE_XOPEN
#include <time.h>

#include "sha1.h"
#include "utils.h"
#include "p16log.h"
#include "hmac.h"

#define PATH_SEP '/'

static uint8 msg_hz_font = HZ_16DZ;
static uint8 msg_asc_font = ASC_16DZ;

uint32 get_time_tick()
{
    struct timeval tv;
    memset(&tv, 0, sizeof tv);
    if (gettimeofday(&tv, NULL))
        return 0;
    return (uint32)tv.tv_sec;
}

void get_std_trans_datetime(const char* input_str, uint8* output, uint32 output_len)
{
//140403103922 ==> '14','4','3','10','39','22'
	int i = 0;
	char tmpstr[2+1];
	uint16 tmpint = 0;
	uint32 offset = 0;
	for(i=0; i<output_len; i++)
	{
		memset(tmpstr, 0, 3);
		memcpy(tmpstr, input_str+offset, 2);
		tmpint = atoi(tmpstr);
		output[i] = (uint8)tmpint;
		offset += 2;
	}
}

void get_order_array_by_int(int input, uint8* output, uint32 out_len)
{
	uint8 tmp_int[5] = {0};
	memcpy(tmp_int,&input,4);
	
	int i = 0;
	for(i=0;i<out_len;i++)
	{
		output[i] = tmp_int[i];
	}
	
}


void encode_hex(uint8* input, uint32 len, char* output)
{
    int i, j;
    for (i = 0, j = 0; i < len; ++i)
    {
        j += sprintf(output + j, "%02X", (uint32)input[i]);
    }
}
void decode_hex(char* input, uint32 len, uint8* output)
{
    int i;
    assert(len % 2 == 0);
    char temp[3];
    memset(temp, 0, sizeof temp);
    for (i = 0; i < len; i += 2)
    {
        memcpy(temp, input + i, 2);
        output[i >> 1] = strtoul(temp, NULL, 16);
    }
}

void bcd_to_hex(const uint8* bcd, uint8 bcd_len, uint8* hex)
{
    uint8 i;
    for (i = 0; i < bcd_len; ++i)
    {
        hex[i] = (((bcd[i] & 0xF0) >> 4) * 10) + (bcd[i] & 0x0F);
    }
}

void hex_to_bcd(const uint8* hex, uint8 len, uint8* bcd)
{
    uint8 i;
    for (i = 0; i < len; ++i)
    {
        bcd[i] = (((hex[i] / 10) << 4) & 0xF0) | ((hex[i] % 10) & 0x0F);
    }
}

void set_disp_msg_font(uint8 hz, uint8 asc)
{
    msg_hz_font = hz;
    msg_asc_font = asc;
}
static void reset_disp_msg_font()
{
    msg_hz_font = DEFAULT_HZ_FONT;
    msg_asc_font = DEFAULT_ASC_FONT;
    CLEAR(MAIN_DISP, 0, DISP_Y_COOR, SCREEN_X_PIXEL, SCREEN_Y_PIXEL);
}
void disp_msg(const char* emsg, uint8 timeout)
{
    uint32 tick = 0;
    uint32 x_coor, y_coor, i;
    const uint32 char_per_line = 20;
    int ret;
    CLEAR_SCREEN;
    dis_mod_set(MAIN_DISP , DEFAULT_HZ_FONT , DEFAULT_ASC_FONT);
    // printf_str(MAIN_DISP, DISP_X_COOR, DISP_Y_COOR + 30, emsg, 1);
    x_coor = DISP_X_COOR;
    y_coor = DISP_Y_COOR + 30;
    char temp[256];
    uint32 offset = 0;
    uint32 msglen = strlen(emsg);
    for (offset = 0; offset < msglen;)
    {
        for (i = 0; i < char_per_line; ++i)
        {
            if (emsg[offset + i] < 128)
                temp[i] = emsg[offset + i];
            else
            {
                if (i + 1 == char_per_line)
                {
                    --i;
                    break;
                }
                else
                {
                    temp[i] = emsg[offset + i];
                }
            }
        }
        offset += i;
        temp[i] = 0;
        //
        printf_str(MAIN_DISP, x_coor, y_coor, temp, 1);
        // LOG((LOG_DEBUG,"line: %s", temp));
        y_coor += 34;
    }
    // dis_mod_set(MAIN_DISP , DEFAULT_HZ_FONT, DEFAULT_ASC_FONT);
    char tip[64];
    lcd_160_upd();
    while (timeout > 0)
    {
        uint32 tick2 = get_time_tick();
        if (tick != tick2)
        {
            --timeout;
            CLEAR(MAIN_DISP, DISP_X_COOR, DISP_Y_COOR + 135, DISP_X_COOR + DISP_X_PIXEL,
                  DISP_Y_PIXEL);
            sprintf(tip, "按键返回,%d秒", timeout);

            printf_str(MAIN_DISP, DISP_X_COOR, DISP_Y_COOR + 140, tip, 1);
            lcd_160_upd();
            tick = tick2;
            // beep(30);
        }
        uint8 keyval;
        ret = get_key(&keyval, TIM_DLY_MOD, 30);
        if (0 == ret)
        {
            // if(ESC_KEY == keyval)
            {
                reset_disp_msg_font();
                return ;
            }
        }
    }
    reset_disp_msg_font();
}

int disp_msg_callback(const char* emsg, uint8 timeout,
                      disp_callback callback, void* userp)
{
    uint32 tick = 0;
    uint32 tick_start = get_time_tick();
    int8 time_left = timeout;
    int ret;
    uint est = 0;
    CLEAR_SCREEN;
    dis_mod_set(MAIN_DISP , DEFAULT_HZ_FONT, DEFAULT_ASC_FONT);
    printf_str(MAIN_DISP, DISP_X_COOR + 3, DISP_Y_COOR + 20, (uint8*)emsg, 1);
    lcd_160_upd();
    while (time_left > 0)
    {
        uint32 tick2 = get_time_tick();
        if (tick != tick2)
        {
            est = tick2 - tick_start;
            time_left = timeout - est;
            char tip[64];
            sprintf(tip, "<取消>返回,%d秒", time_left);
            CLEAR(MAIN_DISP, DISP_X_COOR, DISP_Y_COOR + 80, DISP_X_COOR + DISP_X_PIXEL,
                  SCREEN_Y_PIXEL);
            printf_str(MAIN_DISP, DISP_X_COOR + 3, DISP_Y_COOR + 140, tip, 1);
            lcd_160_upd();
            tick = tick2;
        }
        uint8 keyval;
        ret = get_key(&keyval, TIM_DLY_MOD, 30);
        if (0 == ret)
        {
            if (ESC_KEY == keyval)
            {
                reset_disp_msg_font();
                return -1;
            }
        }
        ret = callback(userp, timeout, est);
        if (!ret)
        {
            reset_disp_msg_font();
            return 0;
        }
    }
    reset_disp_msg_font();
    return -1;
}
void join_path(const char* p1, const char* p2, char* output)
{
    int p1len = strlen(p1);
    if (p1[p1len - 1] == PATH_SEP)
    {
        sprintf(output, "%s%s", p1, p2);
    }
    else
    {
        sprintf(output, "%s%c%s", p1, PATH_SEP, p2);
    }
}

int is_file_exists(const char* file_name)
{
    FILE* fp;
    fp = fopen(file_name, "rb");
    if (!fp)
        return 0;
    fclose(fp);
    return 1;
}

int8 set_local_ip(const char* ip, const char* netmask, const char* gw)
{
    char cmd[512];
    const char eth_name[] = "eth0";
    in_addr_t addr;
    unsigned char* temp;
    sprintf(cmd, "/sbin/ifconfig %s %s netmask %s", eth_name, ip, netmask);
    if (system(cmd))
        return -1;
    // set mac address
    sprintf(cmd, "/sbin/ifconfig %s down", eth_name);
    if (system(cmd))
        return -1;
    addr = inet_addr(ip);
    temp = (unsigned char*)&addr;
    sprintf(cmd, "/sbin/ifconfig %s hw ether 00:AC:%02X:%02X:%02X:%02X",
            eth_name, temp[0], temp[1], temp[2], temp[3]);
    /* LOG((LOG_DEBUG, "CMD: %s" , cmd)); */
    if (system(cmd))
        return -1;
    sprintf(cmd, "/sbin/ifconfig %s up", eth_name);
    if (system(cmd))
        return -1;
    sprintf(cmd, "/sbin/route del default");
    system(cmd);
    sprintf(cmd, "/sbin/route add default gw %s", gw);
    if (system(cmd))
        return -1;
    return 0;
}

int8 check_ip_valid(const char* ip)
{
    int l = strlen(ip);
    int i = 0;
    int j, t;
    for (j = 0; j < 4; ++j)
    {
        if (i >= l)
            break;
        t = 0;
        while (t < 4 && ip[i] != '.' && i < l)
        {
            if (ip[i] >= '0' && ip[i] <= '9')
            {
                ++i;
                ++t;
                continue;
            }
            return -1;
        }
        if (ip[i] != '.' && i < l)
            return -1;
        ++i;
    }
    if (j < 4)
        return -1;
    return 0;
}

void easy_sha1(unsigned char* data,
               unsigned int data_len, char digest[41])
{
    SHA1Context ctx;
    memset(&ctx, 0, sizeof ctx);

    SHA1Reset(&ctx);

    SHA1Input(&ctx, (const unsigned char*)data, data_len);

    SHA1Result(&ctx);
    int i;
    for (i = 0; i < 5; ++i)
    {
        sprintf(digest + (8 * i), "%08X", ctx.Message_Digest[i]);
    }
    digest[40] = '\0';
}

void get_datetime(const char* fmt, char* dt, size_t maxlen)
{
    time_t rawtime;
    struct tm* timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(dt, maxlen, fmt, timeinfo);
}

void easy_hmac(const char* secure_key, unsigned char* data,
               unsigned int data_len, char digest[41])
{
    unsigned char digest_hex[20];
    Hmac hmac;
    memset(&hmac, 0, sizeof hmac);
    HmacSetKey(&hmac, SHA, secure_key, strlen(secure_key));
    HmacUpdate(&hmac, data, data_len);
    HmacFinal(&hmac, digest_hex);
    int i, j;
    for (i = 0, j = 0; i < 20; ++i)
    {
        j += sprintf(digest + j, "%02x", (unsigned int)digest_hex[i]);
    }
}

void get_datetime_offset(uint32 seconds, const char* fmt, char* datetime)
{
    time_t rawtime;
    struct tm* timeinfo;
    time(&rawtime);
    rawtime += seconds;
    timeinfo = localtime(&rawtime);
    strftime(datetime, 16, fmt, timeinfo);
}

static proptey choise_properties[] =
{
    {CHO_STA , DISP_X_COOR, DISP_Y_COOR + 5},
    {CHO_FON_TYPE, DEFAULT_HZ_FONT, DEFAULT_ASC_FONT},
    {CHO_WIDTH, DISP_X_PIXEL, 160},
    // {CHO_BCOR_SET , WHITE_COLR, 0},
    {0xffff ,       0 , 0}
};
int confirm_dialog(const char* hint)
{
    CLEAR_SCREEN;
    choise_dst* input;
    input = choise_init(3);
    if (NULL == input)
    {
        error_exit(2, "创建控件失败");
    }
    choice_props_set(input, choise_properties);
    choise_hind_set(input, (char*)hint);

    choise_dis(MAIN_DISP, input);
    lcd_160_upd();
    int ret = choise_key_manege(MAIN_DISP, input, lcd_160_upd);
    choise_destory(input);
    CLEAR_FULLSCREEN;
    return ret;
}

int confirm_info(const char* hint, uint8* keys)
{
    CLEAR_SCREEN;
    dis_mod_set(MAIN_DISP , msg_hz_font , msg_asc_font);
    printf_str(MAIN_DISP, DISP_X_COOR, DISP_Y_COOR + 30, (uint8*)hint, 1);
    lcd_160_upd();
    int ret, i;
    uint8 key;
    while (1)
    {
        ret = get_key(&key, BLOCK_MOD, -1);
        if (ret == 0)
        {
            for (i = 0; keys[i] != 0; ++i)
            {
                if (keys[i] == key)
                {
                    reset_disp_msg_font();
                    return key;
                }
            }
        }
    }
    reset_disp_msg_font();
    return 0;
}


int convert_datetime(const char* datetime_str, const char* fromfmt, const char* tofmt,
                     char* output, size_t maxlen)
{
    struct tm  t;
    if (strptime(datetime_str, fromfmt, &t) == 0)
    {
        LOG((LOG_DEBUG, "转换时间失败"));
        return -1;
    }
    size_t l = strftime(output, maxlen, tofmt, &t);
    if (l == 0)
        return -1;
    return 0;
}

void encode_device_key(const uint8 key[8], uint8 encrypted[8])
{
    uint8 i;
    // \x32\x30\x30\x35\x30\x31\x33\x31
    const uint8 statickey[] = "\x32\x30\x30\x35\x30\x31\x33\x31";
    for (i = 0; i < 8; ++i)
    {
        encrypted[i] = ((~key[i]) ^ statickey[i]) & 0xFF;
    }
}
int is_number_key(uint8 key, uint8* value)
{
    switch (key)
    {
    case KEY_1:
        *value = 1;
        break;
    case KEY_2:
        *value = 2;
        break;
    case KEY_3:
        *value = 3;
        break;
    case KEY_4:
        *value = 4;
        break;
    case KEY_5:
        *value = 5;
        break;
    case KEY_6:
        *value = 6;
        break;
    case KEY_7:
        *value = 7;
        break;
    case KEY_8:
        *value = 8;
        break;
    case KEY_9:
        *value = 9;
        break;
    case KEY_0:
        *value = 0;
        break;
    default:
        return 0;
    }
    return 1;
}

