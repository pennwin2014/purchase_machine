#ifndef _P16POS_UTILS_H_
#define _P16POS_UTILS_H_

#include <pthread.h>
#include "config.h"

#define SCREEN_X_PIXEL 480
#define DISP_X_PIXEL 285
#define DISP_X_COOR 95
#define DISP_Y_COOR 50
#define SCREEN_Y_PIXEL 272
#define DISP_Y_PIXEL SCREEN_Y_PIXEL
#define INPUT_WIDTH 120
#define INPUT_HEIGHT 34
// 控件 y 向间距
#define CTRL_Y_PAD_PIXEL 5


uint32 get_time_tick();
void encode_hex(uint8* input, uint32 len, char* output);
void decode_hex(char* input, uint32 len, uint8* output);
void bcd_to_hex(const uint8* bcd, uint8 bcd_len, uint8* hex);
void hex_to_bcd(const uint8* hex, uint8 len, uint8* bcd);
void set_disp_msg_font(uint8 hz, uint8 asc);
void disp_msg(const char* emsg, uint8 timeout);
int is_number_key(uint8 key, uint8* value);
void get_order_array_by_int(int input, uint8* output, uint32 out_len);//int转到uint8数组低位在前
void get_std_trans_datetime(const char* input_str, uint8* output, uint32 output_len);//将字符串时间日期转成十六进制


typedef int (* disp_callback)(void* userp, uint8 timeout, uint8 est);
int disp_msg_callback(const char* emsg, uint8 timeout, disp_callback callback,
                      void* userp);

void join_path(const char* p1, const char* p2, char* output);
int8 check_ip_valid(const char* ip);
int8 set_local_ip(const char* ip, const char* netmask, const char* gw);

void encode_device_key(const uint8 key[8], uint8 encrypted[8]);
int is_file_exists(const char* file_name);
/**
 * @brief - sha1包装
 * @param data - 待签名计算数据
 * @param data_len - 待签名数据长度
 * @param degist - 输出签名,16进制
 */
void easy_sha1(unsigned char* data,
               unsigned int data_len, char digest[41]);

/**
 * @brief - sha1 签名包装
 * @param secure_key - 初始值, NULL 表示无初始值
 * @param data - 待签名计算数据
 * @param data_len - 待签名数据长度
 * @param degist - 输出签名,16进制
 */
void easy_hmac(const char* secure_key, unsigned char* data,
               unsigned int data_len, char digest[41]);

void get_datetime(const char* fmt, char* dt, size_t maxlen);

void get_datetime_offset(uint32 seconds, const char* fmt, char* datetime);

int convert_datetime(const char* datetime_str, const char* fromfmt, const char* tofmt,
                     char* output, size_t maxlen);

int confirm_dialog(const char* hint);

int confirm_info(const char* hint, uint8* keys);

#define SAFE_GET_DATETIME(fmt,dt) do{get_datetime(fmt,dt,sizeof(dt)-1); } while(0)

#define SAFE_STR_CPY(d,s) do{ memcpy(d,s,sizeof(d)); }while(0)

// #define CLEAR_SCREEN do { box( MAIN_DISP, 0, 50, 160, 160, 0 ); } while(0)
#define CLEAR_SCREEN do { box_bak( MAIN_DISP, DISP_X_COOR - 10, DISP_Y_COOR - 10, \
                                       SCREEN_X_PIXEL, SCREEN_Y_PIXEL); } while(0)
// DISP_X_COOR + DISP_X_PIXEL + 20, SCREEN_Y_PIXEL); } while(0)

#define CLEAR(disp, x, y, x1, y1) do {box_bak(disp, x, y, x1, y1);} while(0)

#define CLEAR_FULLSCREEN do {box_bak(MAIN_DISP, 0, DISP_Y_COOR, DISP_X_PIXEL, DISP_Y_PIXEL); } while(0)


#endif // _P16POS_UTILS_H_
