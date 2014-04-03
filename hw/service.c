/******************************************************************************
* Copyright SupWisdom 2012 all right reserved
* Module: 调用一卡通系统服务接口
* File: service.c
* Auth: Cheng Tang<cheng.tang@supwisdom.com>
******************************************************************************/

#include <assert.h>
#include <pthread.h>

#include "curl/curl.h"
#include "service.h"
#include "message.h"
#include "cJSON.h"
#include "p16log.h"
#include "syspara.h"
#include "utils.h"
#include "iconv.h"
//以下是新添加的socket部分
#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>





//{{{ global define
#define GLOBAL_SECRET_KEY  "50c7ed94-f44f-4879-a17d-f5962997417f"

#define TEST_DEL_JSON_OBJ(obj) do{ if(obj!=NULL){ cJSON_Delete(obj); } } while(0)

static int svc_timeout = 25;
static char core_svc_url[1024];
static char core_svc_ecardapi_url[1024];

typedef struct
{
    char timestamp[17];
    char method[12];
    char sign_key[65];
} auth_session_t;


enum {STATE_START = 1,
      STATE_CALL,
      STATE_SUCCESS,
      STATE_ERROR,
      STATE_TIMEOUT
     };

enum {API_CALL = 0, ECARD_SVC_CALL = 1};
typedef struct
{
    const char* title;
    uint8 api_call;
    uint8 state;
    int retcode;
    int statuscode;
    cJSON* result;
    const char* url;
    char emsg[512];
    pthread_t thr_id;
    int timeout;
    sw_message_t* request;
    sw_message_t* response;
} call_context_t;

#define LOCAL_ENC "gbk"
#define SVC_ENC "utf-8"
// }}}

static void gen_access_token(auth_session_t* session);

// {{{ 功能函数
void setup_svc_url()
{
    sprintf(core_svc_url, "http://%s:%d%s",
            p16pos.remote[REMOTE_WIRE].u.host.host_name,
            p16pos.remote[REMOTE_WIRE].u.host.host_port,
            p16pos.service_path
           );
    sprintf(core_svc_ecardapi_url, "http://%s:%d%s/ecardservice/ecardapi",
            p16pos.remote[REMOTE_WIRE].u.host.host_name,
            p16pos.remote[REMOTE_WIRE].u.host.host_port,
            p16pos.service_path
           );
    //LOG( ( LOG_DEBUG, "URL: %s", core_svc_url ) );
}

static int iconv_convert(const char* input, uint32 input_len, sw_string_t* output)
{
    iconv_t handler = libiconv_open(LOCAL_ENC, SVC_ENC);
    if (handler == (iconv_t) - 1)
    {
        LOG((LOG_ERROR, "不支持的转码格式"));
        return -1;
    }
    output->use_len = 0;
    char* inptr = (char*)input;
    size_t inptr_left = input_len;
    const size_t outbuff_size = 2048;
    char outbuffer[outbuff_size];
    size_t outbuff_left = outbuff_size;
    char* outptr = outbuffer;
    while (inptr_left > 0)
    {
        memset(outbuffer, 0, sizeof outbuffer);
        outbuff_left = sizeof outbuffer;
        outptr = outbuffer;
        size_t ncvt = libiconv(handler, &inptr, &inptr_left,
                               &outptr, &outbuff_left);
        if (ncvt == (size_t) - 1)
        {
            break;
        }
        ncvt = outbuff_size - outbuff_left;
        if (ncvt > 0)
        {
            //LOG((LOG_DEBUG,"aaaaaaaaaaaaaaaaaaaaaaa"));
            sw_string_cat_cn(output, outbuffer, ncvt);
            //LOG((LOG_DEBUG,"bbbbbbbbbbbbbbbbbbbbbbb"));
        }

    }
    int ret;
    if (inptr_left == 0)
        ret = 0;
    else
        ret = -1;
    libiconv_close(handler);
    return ret;
}

int tcp_init()
{
	
	return 0;
}

static uint16 crc_table[] =
{
  0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
  0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
  0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
  0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
  0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
  0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
  0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
  0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
  0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
  0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
  0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
  0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
  0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
  0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
  0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
  0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
  0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
  0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
  0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
  0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
  0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
  0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
  0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
  0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
  0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
  0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
  0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
  0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
  0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
  0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
  0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
  0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};

void pb_protocol_crc(const uint8* buffer, uint16 buffer_len, 
                     uint8 crc[2])
{
  uint16 accnum = 0xc78c;
  uint16 i;
  for(i = 0; i < buffer_len; ++i)
  {
    accnum = (accnum << 8) ^ crc_table[((accnum >> 8)
                             ^ buffer[i]) & 0xFF];
  }
  crc[0] = (uint8)((accnum >> 8) & 0xFF);
  crc[1] = (uint8)(accnum & 0xFF);
}

uint8 calc_check_sum(uint8* check_data,uint32 check_len)
{
	uint32 tmp_sum = 0;
	uint8 tmp_result = 0;
	int i = 0;
	for(i=0;i<check_len;i++)
	{
		tmp_sum += check_data[i];
	}
	tmp_result = (uint8)tmp_sum;
	tmp_result = ~tmp_result;
	return tmp_result;
}


void pack_tcp_heart_data(p16_tcp_heart_package* package, char* send_buf, uint32* send_len)
{
	//发送的字符数组赋值
	uint32 f = 0;
	memcpy(send_buf+f, package->guide_code, 3);//引导码
	f += 3;
	send_buf[f] = package->cmd_code;//命令码
	f += 1;
	//由于需要低位在前的处理
	uint8 packege_len[2];
	memcpy(packege_len, &(package->package_len), 2);
	memcpy(send_buf+f, packege_len, 2);//数据包长度(低位在前)
	f += 2;
	memcpy(send_buf+f, package->machine_addr,2);//机器地址
	f += 2;
	send_buf[f] = package->data.identify_code;
	f += 1;
	memcpy(send_buf+f, package->data.machine_no, 4);//机号
	f += 4;
	memcpy(send_buf+f, package->data.cfgverno, 4);//设备参数版本号
	f += 4;
	package->check = calc_check_sum(send_buf,f);
	send_buf[f] = package->check;//校验
	f += 1;
	(*send_len) = f;
}


void pack_tcp_blacklist_data(p16_tcp_blacklist_package* package, char* send_buf, uint32* send_len)
{
	//发送的字符数组赋值
	uint32 f = 0;
	memcpy(send_buf+f, package->guide_code, 3);//引导码
	f += 3;
	send_buf[f] = package->cmd_code;//命令码
	f += 1;
	//由于需要低位在前的处理
	uint8 packege_len[2];
	memcpy(packege_len, &(package->package_len), 2);
	memcpy(send_buf+f, packege_len, 2);//数据包长度(低位在前)
	f += 2;
	memcpy(send_buf+f, package->machine_addr,2);//机器地址
	f += 2;
	memcpy(send_buf+f, package->data.pos_blacklist_version,6);//
	f += 6;
	memcpy(send_buf+f, package->data.machine_no,4);//
	f += 4;
	package->check = calc_check_sum(send_buf,f);
	send_buf[f] = package->check;//校验
	f += 1;
	(*send_len) = f;

}

void pack_tcp_rt_transdtl_data(p16_tcp_rt_package* package, char* send_buf, uint32* send_len)
{
	//发送的字符数组赋值
	uint32 f = 0;
	memcpy(send_buf+f, package->guide_code, 3);//引导码
	f += 3;
	send_buf[f] = package->cmd_code;//命令码
	f += 1;
	//由于需要低位在前的处理
	uint8 packege_len[2];
	get_order_array_by_int(package->package_len,packege_len,2);
	memcpy(send_buf+f, packege_len, 2);//数据包长度(低位在前)
	f += 2;
	memcpy(send_buf+f, package->machine_addr,2);//机器地址
	f += 2;
	uint32 data_size = 0;
	data_size = sizeof(package->data);//data里的数据
	memcpy(send_buf+f, &package->data, data_size);
	f += data_size;
	package->check = calc_check_sum(send_buf,f);
	send_buf[f] = package->check;//校验
	f += 1;
	(*send_len) = f;
}

void pack_tcp_batch_data(p16_tcp_batch_package* batch_package, char* send_buf, uint32* send_len)
{
	//发送的字符数组赋值
	uint32 f = 0;
	memcpy(send_buf+f, batch_package->guide_code, 3);//引导码
	f += 3;
	send_buf[f] = batch_package->cmd_code;//命令码
	f += 1;
	//由于需要低位在前的处理
	uint8 packege_len[2];
	memcpy(packege_len, &(batch_package->package_len), 2);
	memcpy(send_buf+f, packege_len, 2);//数据包长度(低位在前)
	LOG((LOG_DEBUG,"in function pack_tcp_send_data::package_len=%d",batch_package->package_len));
	f += 2;
	memcpy(send_buf+f, batch_package->machine_addr,2);//机器地址
	f += 2;
	send_buf[f] = batch_package->data.tcp_transdtl_cnt;//data里的条数
	f += 1;
	uint32 data_size = 0;
	data_size = batch_package->data.tcp_transdtl_cnt * sizeof(p16_tcp_transdtl);//data里的数据
	memcpy(send_buf+f, batch_package->data.tcp_transdtl_list, data_size);
	f += data_size;
	batch_package->check = calc_check_sum(send_buf,f);
	send_buf[f] = batch_package->check;//校验
	f += 1;
	(*send_len) = f;
}

int tcp_send_data(char* send_buf, uint32 send_len, char* recv_buf, uint32* recv_len)//发送函数
{
	int cfd;
	int recbytes,writebytes;
	struct sockaddr_in s_add;
	unsigned short portnum=p16pos.remote[REMOTE_WIRE].u.host.host_port; 

	cfd = socket(AF_INET, SOCK_STREAM, 0);
	if(-1 == cfd)
	{
	    LOG((LOG_DEBUG,"socket fail ! \r\n"));
	    return -1;
	}

	bzero(&s_add,sizeof(struct sockaddr_in));
	s_add.sin_family=AF_INET;
	s_add.sin_addr.s_addr= inet_addr(p16pos.remote[REMOTE_WIRE].u.host.host_name);
	s_add.sin_port=htons(portnum);

	//设置超时时间
	struct timeval tv;
	tv.tv_sec = 5;  //5秒超时
	tv.tv_usec = 0;  
	setsockopt(cfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));

	if(-1 == connect(cfd,(struct sockaddr *)(&s_add), sizeof(struct sockaddr)))
	{
	    LOG((LOG_DEBUG,"connect fail !\r\n"));
	    return -1;
	}
	LOG((LOG_DEBUG,"connect ok !"));
	
    if((writebytes = write(cfd,send_buf,send_len))<0)
    {
		LOG((LOG_DEBUG,"write fail !\r\n"));
	}
	LOG((LOG_DEBUG,"write ok !"));

	if((recbytes = read(cfd,recv_buf,1024)) < 0)
	{
		LOG((LOG_DEBUG,"read data fail !recbytes=%d\r\n",recbytes));
		close(cfd);
	    return -1;
	}
	LOG((LOG_DEBUG,"read ok !\r\n接收到数据打印:"));
	recv_buf[recbytes]='\0';
	(*recv_len) = recbytes;
	p16dump_hex(LOG_DEBUG,recv_buf,recbytes);
	close(cfd);
	return 0;
}

int tcp_demo_send()
{
	int cfd;
	int recbytes;
	char buffer[1024]={0};   
	struct sockaddr_in s_add;
	unsigned short portnum=0x8888; 
	LOG((LOG_DEBUG,"Hello,welcome to client !\r\n"));

	cfd = socket(AF_INET, SOCK_STREAM, 0);
	if(-1 == cfd)
	{
	    LOG((LOG_DEBUG,"socket fail ! \r\n"));
	    return -1;
	}
	printf("socket ok !\r\n");

	bzero(&s_add,sizeof(struct sockaddr_in));
	s_add.sin_family=AF_INET;
	s_add.sin_addr.s_addr= inet_addr("192.168.1.148");
	s_add.sin_port=htons(portnum);
	printf("s_addr = %#x ,port : %#x\r\n",s_add.sin_addr.s_addr,s_add.sin_port);

	if(-1 == connect(cfd,(struct sockaddr *)(&s_add), sizeof(struct sockaddr)))
	{
	    LOG((LOG_DEBUG,"connect fail !\r\n"));
	    return -1;
	}
	LOG((LOG_DEBUG,"connect ok !\r\n"));

	if(-1 == (recbytes = read(cfd,buffer,1024)))
	{
		LOG((LOG_DEBUG,"read data fail !\r\n"));
	    return -1;
	}
	LOG((LOG_DEBUG,"read ok\r\nREC:\r\n"));
	buffer[recbytes]='\0';
	LOG((LOG_DEBUG,"%s\r\n",buffer));
	//getchar();
	close(cfd);
	return 0;
}


static int _get_cJSON_string_value(cJSON* obj, char* value, int maxlen)
{
    assert(obj != NULL);
    switch (obj->type)
    {
    case cJSON_False:
        strncpy(value, "false", maxlen);
        break;
    case cJSON_True:
        strncpy(value, "true", maxlen);
        break;
    case cJSON_NULL:
        strncpy(value, "NULL", maxlen);
        break;
    case cJSON_Number:
        sprintf(value, "%d", obj->valueint);
        break;
    case cJSON_String:
        memcpy(value, obj->valuestring, maxlen);
        break;
    case cJSON_Array:
        return -1;
    case cJSON_Object:
        return -1;
    }
    return 0;
}
static int easy_json_value(cJSON* root, const char* name, char* value, int maxlen)
{
    cJSON* elem = cJSON_GetObjectItem(root, name);
    if (NULL == elem)
        return -1;
    return _get_cJSON_string_value(elem, value, maxlen);
}
#define JSON_SAFE_STRING(r,n,v) easy_json_value(r,n,v,sizeof(v)-1)

// }}}

//{{{ curl 相关功能
static size_t curl_write_data(void* buf, size_t size, size_t nmemb, sw_string_t* userp)
{
    size_t total = size * nmemb;
    if (sw_string_cat_cn(userp, (const char*)buf, total))
    {
        LOG((LOG_ERROR, "创建失败"));
        return 0;
    }
    return total;
}
typedef struct
{
    uint32 start;
    uint32 tick;
    int timeout;
} curl_process_data_t;

static int curl_process_func(void* clientp,
                             double dltotal,
                             double dlnow,
                             double ultotal,
                             double ulnow)
{
#if 0
    curl_process_data_t* data = (curl_process_data_t*)clientp;
    uint32 current = get_time_tick();
    if (data->tick != current)
    {
        CLEAR_SCREEN;

        data->tick = current;
        uint32 est = current - data->start;
        if (ulnow >= 0.0 && dlnow <= 0.001)
            printf_str(MAIN_DISP, 3, 60, "等待发送数据", 1);
        else if (dlnow > 0.0)
            printf_str(MAIN_DISP, 3, 60, "等待接收数据", 1);

    }
#endif
    return 0;
}
/**
 * @brief - CURL 请求后台功能
 * @param curl - curl 句柄
 * @param url - 请求地址
 * @param req - 请求数据， 为 NULL 时用GET方法请求，!=NULL 时用POST方法请求
 * @param resp - 应答数据结果
 * @param timeout - 超时时间
 * @return - 返回 0 表示成功，-1 表示请求失败，-2 表示请求超时
 */
static int curl_callsvr(CURL* curl, const char* url, sw_string_t* req, sw_string_t* resp,
                        int timeout)
{
    char error_buffer[CURL_ERROR_SIZE] = {0};
    curl_process_data_t data;
    data.start = get_time_tick();
    data.tick = data.start;
    data.timeout = timeout;
    sw_string_t respdata;
    memset(&respdata, 0, sizeof respdata);
    sw_new_string(&respdata);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &curl_write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &respdata);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error_buffer);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
    curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, &curl_process_func);
    curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &data);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
    struct curl_slist* headerlist = NULL;
    if (req != NULL)  // POST
    {
        //
        //LOG((LOG_DEBUG,"url: %s",url));
        headerlist = curl_slist_append(headerlist, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, req->str);
        curl_easy_setopt(curl, CURLOPT_POST, 1);
        //printf("[%s]\n",req->str);
    }


    CURLcode code;

    code = curl_easy_perform(curl);
    curl_slist_free_all(headerlist);
    if (CURLE_OK != code)
    {
        sw_free_string(&respdata);
        LOG((LOG_ERROR, "CURL请求后台错误: %s", error_buffer));
        LOG((LOG_DEBUG, "URL: %s", url));
        if (CURLE_OPERATION_TIMEDOUT == code)
            return SVC_TIMEOUT;
        return SVC_FAILED;
    }
    else
    {

        if (iconv_convert(respdata.str, respdata.use_len, resp))
        {
            LOG((LOG_ERROR, "转码编码失败"));
            sw_free_string(&respdata);
            return SVC_FAILED;
        }

        sw_string_finalize(resp);
    }

    return SVC_OK;
}


static int curl_callsvr_proxy(const char* url, int* code, sw_string_t* req,
                              sw_string_t* resp, int timeout)
{
    CURL* curl_handler = curl_easy_init();
    int ret = curl_callsvr(curl_handler, url, req, resp , timeout);
    if (!ret)
    {
        long resp_code = 0;
        CURLcode rc = curl_easy_getinfo(curl_handler, CURLINFO_RESPONSE_CODE, &resp_code);
        if (rc != CURLE_OK)
        {
            return -1;
        }
        *code = (int)resp_code;
    }
    curl_easy_cleanup(curl_handler);
    if (ret == SVC_TIMEOUT)
    {
        p16pos.last_trans_timeout = 1;
    }
    else
    {
        p16pos.last_trans_timeout = 0;
    }
    return ret;
}


static int ecard_call_parameter(sw_message_t* msg, sw_string_t* result)
{
    int ret;
    sw_string_t msgstr;
    sw_new_string(&msgstr);
    ret = sw_message_serialize(msg, &msgstr);
    if (ret)
    {
        sw_free_string(&msgstr);
        return -1;
    }

    sw_string_finalize(&msgstr);

    cJSON* root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "app_id", p16pos.appid);
    cJSON_AddStringToObject(root, "term_id", p16pos.devphyid);

    auth_session_t auth;
    memset(&auth, 0, sizeof auth);
    gen_access_token(&auth);
    cJSON_AddStringToObject(root, "timestamp", auth.timestamp);
    cJSON_AddStringToObject(root, "sign_method", auth.method);
    cJSON_AddStringToObject(root, "sign", auth.sign_key);
    cJSON_AddStringToObject(root, "funcdata", msgstr.str);


    sw_string_copy_c(result, cJSON_PrintUnformatted(root));
    return 0;
}
static void ecard_callsvr(void* arg)
{
    int ret;
    call_context_t* ctx = (call_context_t*)arg;
    assert(ctx->request != NULL && ctx->response != NULL);
    sw_string_t req, resp;
    sw_new_string(&req);

    ret = ecard_call_parameter(ctx->request, &req);
    if (ret)
    {
        sw_free_string(&req);
        LOG((LOG_ERROR, "组织请求报文失败"));
        strcpy(ctx->emsg, "组织请求报文失败");
        ctx->state = STATE_ERROR;
        ctx->retcode = ret;
        pthread_exit(0);
        return;
    }
    ctx->state = STATE_CALL;
    sw_new_string(&resp);
    // LOG((LOG_DEBUG, "aaaaaaaaaaaaaaaaaaaaaaaaa"));
    ret = curl_callsvr_proxy(core_svc_ecardapi_url, &ctx->statuscode, &req, &resp,
                             ctx->timeout);
    // LOG((LOG_DEBUG, "bbbbbbbbbbbbbbbbbbbbbbbbb"));
    sw_free_string(&req);
    if (ret)
    {
        sw_free_string(&resp);
        strcpy(ctx->emsg, "请求后台异常");
        ctx->state = STATE_ERROR;
        ctx->retcode = ret;
        pthread_exit(0);
        return;
    }
    //LOG((LOG_DEBUG,"ccccccccccccccccccccccccccccc"));
    ret = sw_message_unserialize(&resp, ctx->response);
    //LOG((LOG_DEBUG,"ddddddddddddddddddddddddddddd"));
    sw_free_string(&resp);
    if (ret)
    {
        strcpy(ctx->emsg, "返回数据包错误");
        ctx->state = STATE_ERROR;
        ctx->retcode = SVC_FAILED;
    }
    else
    {
        ctx->state = STATE_SUCCESS;
        ctx->retcode = 0;
    }
    pthread_exit(0);
}

static void api_callsvr(void* arg)
{
    int ret;
    call_context_t* ctx = (call_context_t*)arg;
    sw_string_t resp;
    sw_new_string(&resp);
    ret = curl_callsvr_proxy(ctx->url, &ctx->statuscode, NULL, &resp, ctx->timeout);
    if (ret)
    {
        strcpy(ctx->emsg, "请求后台异常");
        ctx->state = STATE_ERROR;
        ctx->retcode = -1;
        pthread_exit(0);
        return;
    }
    if (resp.use_len < 2)
    {
        strcpy(ctx->emsg, "后台数据异常");
        ctx->state = STATE_ERROR;
        ctx->retcode = -1;
        pthread_exit(0);
        return;
    }
    sw_string_finalize(&resp);
    LOG((LOG_DEBUG, "请求返回:http status code : %d", ctx->statuscode));
    cJSON* c = cJSON_Parse(resp.str);
    if (NULL == c)
    {
        strcpy(ctx->emsg, "后台数据异常");
        ctx->state = STATE_ERROR;
        ctx->retcode = -1;
        pthread_exit(0);
        return;
    }
    ctx->result = c;
    ctx->state = STATE_SUCCESS;
    ctx->retcode = 0;
    pthread_exit(0);
    return;
}

static int thread_svc_callsvr(call_context_t* ctx, int timeout)
{
    uint32 start, current;
    int ret;
    ctx->timeout = timeout;
    if (ctx->api_call == API_CALL)
        ret = pthread_create(&ctx->thr_id, NULL, (void*)api_callsvr, ctx);
    else
        ret = pthread_create(&ctx->thr_id, NULL, (void*)ecard_callsvr, ctx);
    if (ret)
    {
        LOG((LOG_ERROR, "创建worker线程失败"));
        return SVC_FAILED;
    }
    start = get_time_tick();
    CLEAR_SCREEN;
    dis_mod_set(MAIN_DISP, DEFAULT_HZ_FONT, DEFAULT_ASC_FONT);
    while (ctx->state < STATE_SUCCESS && timeout > 0)
    {
        current = get_time_tick();
        if (current != start)
        {
            char tip[64];
            start = current;
            CLEAR(MAIN_DISP, DISP_X_COOR, DISP_Y_COOR, DISP_X_COOR + DISP_X_PIXEL, DISP_Y_COOR + 65);
            printf_str(MAIN_DISP, DISP_X_COOR, DISP_Y_COOR + 5, (uint8*)ctx->title, 0);
            sprintf(tip, "等待%d秒", --timeout);
            printf_str(MAIN_DISP, DISP_X_COOR, DISP_Y_COOR + 35, tip, 0);
            lcd_160_upd();
        }
        if (ctx->state >= STATE_SUCCESS)
            break;
    }
    pthread_join(ctx->thr_id, NULL);
    if (ctx->state < STATE_SUCCESS)
    {
        ctx->retcode = SVC_FAILED;
    }
    return ctx->retcode;
}
static int svc_get_auth_token(char* token)
{
    sw_string_t url;
    memset(&url, 0, sizeof url);
    sw_string_new_size(&url, 2048);
    sw_string_copy_c(&url, core_svc_url);
    sw_string_cat_c(&url, "/authservice/getauth/");
    sw_string_cat_c(&url, p16pos.appid);
    sw_string_cat_c(&url, "/getaccesstoken?term_id=");
    sw_string_cat_c(&url, p16pos.devphyid);
    sw_string_finalize(&url);

    call_context_t ctx;
    memset(&ctx, 0, sizeof ctx);
    int ret;
    char access_token[65] = {0};
    char emsg[512];
    ctx.url = url.str;
    ctx.api_call = API_CALL;
    ctx.title = "APP申请签到";
    if ((ret = thread_svc_callsvr(&ctx, svc_timeout)))
    {
        disp_msg("APP签到失败", 10);
        ret = -1;
    }
    else if (ctx.statuscode != 200)
    {
        ret = -1;
        sprintf(emsg, "取token失败：%d", ctx.statuscode);
        disp_msg(emsg, 10);
    }
    else if (JSON_SAFE_STRING(ctx.result, "access_token", access_token))
    {
        ret = -1;
        disp_msg("未取到令牌", 10);
    }
    else
    {
        strcpy(token, access_token);
        ret = 0;
    }
    sw_free_string(&url);
    TEST_DEL_JSON_OBJ(ctx.result);
    LOG((LOG_DEBUG, "end get access_token"));
    return ret;
}
// }}}
// {{{ 签到相关函数
static void gen_access_token(auth_session_t* session)
{
    SAFE_GET_DATETIME("%Y%m%d%H%M%S", session->timestamp);
    sw_string_t sign_data;
    sw_new_string(&sign_data);
    sw_string_copy_c(&sign_data, p16pos.appid);
    sw_string_cat_c(&sign_data, p16pos.devphyid);
    sw_string_cat_c(&sign_data, p16pos.session_key);
    sw_string_cat_c(&sign_data, session->timestamp);

    easy_hmac(p16pos.appsecret, sign_data.str, sign_data.use_len, session->sign_key);
    strcpy(session->method, "HMAC");
    sw_free_string(&sign_data);
}

int svc_app_login()
{

    int ret;
    char access_token[128] = {0};
    // 获取认证access token
    ret = svc_get_auth_token(access_token);
    if (ret)
    {
        return -1;
    }
    LOG((LOG_DEBUG, "access token: %s", access_token));
    // 认证获取 session key
    sw_string_t url;
    memset(&url, 0, sizeof url);
    sw_string_new_size(&url, 2048);
    sw_string_copy_c(&url, core_svc_url);
    sw_string_cat_c(&url, "/authservice/getauth/");
    sw_string_cat_c(&url, p16pos.appid);
    sw_string_cat_c(&url, "?access_token=");
    sw_string_cat_c(&url, access_token);
    sw_string_cat_c(&url, "&term_id=");
    sw_string_cat_c(&url, p16pos.devphyid);
    sw_string_cat_c(&url, "&sign_method=HMAC");

    char timestamp[17] = {0};
    SAFE_GET_DATETIME("%Y%m%d%H%M%S", timestamp);
    //LOG((LOG_DEBUG,"timestamp : %s",timestamp));
    strcat(access_token, timestamp);
    char sign[65] = {0};
    //LOG((LOG_DEBUG,"HMAC key: %s",p16pos.appsecret));
    //LOG((LOG_DEBUG,"HMAC data: %s",access_token));
    easy_hmac(p16pos.appsecret, access_token, strlen(access_token), sign);
    //LOG((LOG_DEBUG,"HMAC sign: %s",sign));
    sw_string_cat_c(&url, "&sign=");
    sw_string_cat_c(&url, sign);
    sw_string_cat_c(&url, "&timestamp=");
    sw_string_cat_c(&url, timestamp);
    sw_string_cat_c(&url, "&v=1");
    sw_string_finalize(&url);

    // disp_context_t* disp = disp_new_context();
    // disp_message(disp, "等待操作员签到", svc_timeout + 1);

    //LOG((LOG_DEBUG,"URL: %s",url.str));
    call_context_t ctx;
    char emsg[512];
    memset(&ctx, 0, sizeof ctx);
    ctx.url = url.str;
    ctx.title = "等待应用签到";
    ctx.api_call = API_CALL;
    if ((ret = thread_svc_callsvr(&ctx, svc_timeout)))
    {
        // disp_wait_message_timeout(disp);
        disp_msg("APP认证失败", 10);
        ret = -1;
    }
    else if (ctx.statuscode != 200)
    {
        sprintf(emsg, "认证失败：%d", ctx.statuscode);
        disp_msg(emsg, 10);
        ret = -1;
    }
    else if (JSON_SAFE_STRING(ctx.result, "session_key", p16pos.session_key))
    {
        disp_msg("未取到session_key", 10);
        ret = -1;
    }
    else
    {
        char hex_device_key[33];
        uint8 device_key[8];
        JSON_SAFE_STRING(ctx.result, "card_key", hex_device_key);
        // LOG((LOG_DEBUG, "devicekey[%s]", hex_device_key));
        decode_hex(hex_device_key, 16, device_key);
        // p16dump_hex(LOG_DEBUG, device_key, 8);
        encode_device_key(device_key, p16pos.device_key);
        // p16dump_hex(LOG_DEBUG, p16pos.device_key, 8);
        ret = 0;
    }
    sw_free_string(&url);
    TEST_DEL_JSON_OBJ(ctx.result);
    LOG((LOG_DEBUG, "login session key :%s", p16pos.session_key));
    return ret;
}

int svc_oper_login(const char* cardphyno, const char* operpwd, int* cfgverno)
{
    const char* colnames[] =
    {
        "cardphyid",
        "devphyid",
        "cardpwd",
    };
    sw_message_t reader, writer;
    memset(&reader, 0, sizeof reader);
    memset(&writer, 0, sizeof writer);
    sw_message_reset(&reader);
    sw_message_reset(&writer);
    sw_message_col_define(&writer, colnames, sizeof(colnames) / sizeof(colnames[0]));
    writer.funcno = 950070;

    sw_message_set_row(&writer, "cardphyid", cardphyno);
    sw_message_set_row(&writer, "cardpwd", operpwd);
    sw_message_set_row(&writer, "devphyid", p16pos.devphyid);
    sw_message_add_row(&writer);

    // disp_message(disp_ctx, "等待操作员签到", 20);
    call_context_t ctx;
    memset(&ctx, 0, sizeof ctx);
    ctx.title = "等待操作员签到";
    ctx.request = &writer;
    ctx.response = &reader;
    ctx.api_call = ECARD_SVC_CALL;
    int ret;

    if ((ret = thread_svc_callsvr(&ctx, svc_timeout)))
    {
        LOG((LOG_ERROR, "请求后台错误，[%s]", ctx.emsg));
        disp_msg("请求后台错误", 10);
    }
    else if (reader.retcode != 0)
    {
        LOG((LOG_ERROR, "请求错误：ret=%d,msg=%s", reader.retcode, reader.retmsg.str));
        char tip[280];
        sprintf(tip, "签到失败,%d %s", reader.retcode , reader.retmsg.str);
        disp_msg(tip, 10);
        ret = -2;
    }
    else if (!sw_message_fetch_row(&reader))
    {
        // 返回批次号，操作员号
        sw_message_get_row_int(&reader, "operid", &(p16pos.operid));
        sw_message_get_row_int(&reader, "batchno", &(p16pos.batchno));
        sw_message_get_row_int(&reader, "authcode", &(p16pos.authcode));
        sw_message_get_row_int(&reader, "termid", &(p16pos.termid));
        sw_message_get_row_int(&reader, "termseqno", &(p16pos.termseqno));
        sw_message_get_row_int(&reader, "cfgverno", cfgverno);
        SW_MSG_SAFE_GET_ROW(&reader, "opername", p16pos.opername);
        p16pos.login_time_tick = get_time_tick();
        LOG((LOG_DEBUG, "签到成功,oper=%d,seqno=%d,batchno=%d", p16pos.operid, p16pos.termseqno,
             p16pos.batchno));

        ret = 0;
    }
    else
    {
        disp_msg("系统返回错误", 10);
        ret = -1;
    }
    sw_message_free(&writer);
    sw_message_free(&reader);
    return ret;
}
//}}}
// {{{ 充值及充值查询函数
static int do_svc_deposit(p16_transdtl_t* transdtl, int flag)
{
    const char* colnames[] =
    {
        "cardphyid",
        "devphyid",
        "samno",
        "cardno",
        "cardbefbal",
        "paycnt",
        "dpscnt",
        "amount",
        "random",
        "dpsamt",
        "mac1",
    };
    sw_message_t reader, writer;
    memset(&reader, 0, sizeof reader);
    memset(&writer, 0, sizeof writer);
    sw_message_reset(&reader);
    sw_message_reset(&writer);
    sw_message_col_define(&writer, colnames, sizeof(colnames) / sizeof(colnames[0]));
    writer.funcno = 950071;
    sw_message_set_attr(&writer, "termdate", transdtl->transdate);
    sw_message_set_attr(&writer, "termtime", transdtl->transtime);
    sw_message_set_attr_int(&writer, "authcode", transdtl->authcode);
    sw_message_set_attr_int(&writer, "batchno", transdtl->batchno);
    sw_message_set_attr_int(&writer, "operid", transdtl->operid);
    sw_message_set_attr_int(&writer, "termseqno", transdtl->devseqno);
    sw_message_set_attr_int(&writer, "termid", transdtl->termid);
    sw_message_set_attr_int(&writer, "retry", flag);
    if (flag == FLAG_QUERY)
    {
        sw_message_set_attr(&writer, "reqflag", "Q");
    }

    sw_message_set_row(&writer, "cardphyid", transdtl->cardphyid);
    sw_message_set_row(&writer, "devphyid", transdtl->devphyid);
    sw_message_set_row(&writer, "samno", transdtl->termno);
    sw_message_set_row_int(&writer, "cardno", transdtl->cardno);
    sw_message_set_row_int(&writer, "cardbefbal", transdtl->cardbefbal);
    sw_message_set_row_int(&writer, "paycnt", transdtl->paycnt);
    sw_message_set_row_int(&writer, "dpscnt", transdtl->dpscnt);
    sw_message_set_row_int(&writer, "amount", transdtl->amount);
    sw_message_set_row_int(&writer, "dpsamt", transdtl->dpsamt);
    sw_message_set_row(&writer, "random", transdtl->random);
    sw_message_set_row(&writer, "mac1", transdtl->mac1);

    sw_message_add_row(&writer);

    call_context_t ctx;
    memset(&ctx, 0, sizeof ctx);
    ctx.request = &writer;
    ctx.response = &reader;
    ctx.api_call = ECARD_SVC_CALL;
    if (flag == 0)
    {
        ctx.title = "等待发送圈存请求";
    }
    else
    {
        ctx.title = "等待发送查询请求";
    }
    int ret;
    if ((ret = thread_svc_callsvr(&ctx, svc_timeout)))
    {
        LOG((LOG_ERROR, "请求后台错误，%s", ctx.emsg));
        disp_msg("请求后台错误", 10);
    }
    else if (reader.retcode != 0)
    {
        LOG((LOG_ERROR, "请求错误：ret=%d,msg=%s", reader.retcode, reader.retmsg.str));
        SAFE_STR_CPY(transdtl->errmsg, reader.retmsg.str);
        transdtl->errcode = reader.retcode;
        ret = SVC_FAILED;
    }
    else if (!sw_message_fetch_row(&reader))
    {
        // 返回批次号，操作员号
        SW_MSG_SAFE_GET_ROW(&reader, "mac2", transdtl->mac2);
        SW_MSG_SAFE_GET_ROW(&reader, "hostdate", transdtl->hostdate);
        SW_MSG_SAFE_GET_ROW(&reader, "hosttime", transdtl->hosttime);
        SW_MSG_SAFE_GET_ROW(&reader, "refno", transdtl->refno);
        ret = 0;
    }
    else
    {
        disp_msg("系统充值返回错误", 10);
        ret = SVC_FAILED;
    }
    sw_message_free(&writer);
    sw_message_free(&reader);
    return ret;
}
int svc_deposit(p16_transdtl_t* transdtl)
{
    return do_svc_deposit(transdtl , FLAG_TRANSACTION);
}

int svc_deposit_query(p16_transdtl_t* transdtl)
{
    return do_svc_deposit(transdtl , FLAG_QUERY);
}
// }}}
// {{{ svc_deposit_confirm function
int svc_deposit_confirm(p16_transdtl_t* transdtl, int* nexttermseqno)
{
    const char* colnames[] =
    {
        "cardno",
        "refno",
        "updtype",
        "isok",
        "isbreak",
        "tac",
    };
    sw_message_t reader, writer;
    memset(&reader, 0, sizeof reader);
    memset(&writer, 0, sizeof writer);
    sw_message_reset(&reader);
    sw_message_reset(&writer);
    sw_message_col_define(&writer, colnames, sizeof(colnames) / sizeof(colnames[0]));
    writer.funcno = 846340;
    sw_message_set_attr(&writer, "termdate", transdtl->transdate);
    sw_message_set_attr(&writer, "termtime", transdtl->transtime);
    sw_message_set_attr_int(&writer, "authcode", transdtl->authcode);
    sw_message_set_attr_int(&writer, "batchno", transdtl->batchno);
    sw_message_set_attr_int(&writer, "operid", transdtl->operid);
    sw_message_set_attr_int(&writer, "termseqno", transdtl->devseqno);
    sw_message_set_attr_int(&writer, "termid", transdtl->termid);

    sw_message_set_row_int(&writer, "cardno", transdtl->cardno);
    sw_message_set_row(&writer, "refno", transdtl->refno);
    sw_message_set_row(&writer, "updtype", "cardbal");
    if (TRANS_WRITE_CARD_OK == transdtl->transflag)
    {
        sw_message_set_row_int(&writer, "isok", 1);
        sw_message_set_row_int(&writer, "isbreak", 0);
        char tac[9];
        encode_hex(transdtl->tac, 4, tac);
        sw_message_set_row(&writer, "tac", tac);
    }
    else if (TRANS_WRITE_CARD_FAILED == transdtl->transflag)
    {
        sw_message_set_row_int(&writer, "isok", 0);
        sw_message_set_row_int(&writer, "isbreak", 0);
    }
    else
    {
        sw_message_set_row_int(&writer, "isok", 0);
        sw_message_set_row_int(&writer, "isbreak", 1);
    }
    sw_message_add_row(&writer);


    call_context_t ctx;
    memset(&ctx, 0, sizeof ctx);
    ctx.title = "等待发送确认请求";
    ctx.request = &writer;
    ctx.response = &reader;
    ctx.api_call = ECARD_SVC_CALL;
    int ret;
    if ((ret = thread_svc_callsvr(&ctx, svc_timeout)))
    {
        LOG((LOG_ERROR, "请求后台错误，%s", ctx.emsg));
        disp_msg("请求后台错误", 10);
    }
    else if (reader.retcode != 0)
    {
        LOG((LOG_ERROR, "请求错误：ret=%d", reader.retcode));
        SAFE_STR_CPY(transdtl->errmsg, reader.retmsg.str);
        transdtl->errcode = reader.retcode;
        ret = SVC_FAILED;
    }
    else if (!sw_message_fetch_row(&reader))
    {
        // 返回批次号，操作员号
        sw_message_get_row_int(&reader, "nexttermseqno", nexttermseqno);
        ret = 0;
    }
    else
    {
        disp_msg("系统充值返回错误", 10);
        ret = SVC_FAILED;
    }
    sw_message_free(&reader);
    sw_message_free(&writer);
    return ret;
}
// }}}
// {{{ svc_get_download_dps_para function
int svc_get_download_dps_para(const char* devphyid)
{
    const char* colnames[] =
    {
        "devphyid",
    };
    sw_message_t reader, writer;
    memset(&reader, 0, sizeof reader);
    memset(&writer, 0, sizeof writer);
    sw_message_reset(&reader);
    sw_message_reset(&writer);
    sw_message_col_define(&writer, colnames, sizeof(colnames) / sizeof(colnames[0]));
    writer.funcno = 950073;

    sw_message_set_row(&writer, "devphyid", devphyid);
    sw_message_add_row(&writer);

    // disp_message(disp, "等待下载充值参数", svc_timeout + 1);
    call_context_t ctx;
    memset(&ctx, 0, sizeof ctx);
    ctx.title = "等待下载充值参数";
    ctx.request = &writer;
    ctx.response = &reader;
    ctx.api_call = ECARD_SVC_CALL;
    int ret;
    char emsg[512] = {0};
    if ((ret = thread_svc_callsvr(&ctx, svc_timeout)))
    {
        LOG((LOG_ERROR, "请求后台错误，%s", ctx.emsg));
        disp_msg("请求后台错误", 10);
    }
    else if (reader.retcode != 0)
    {
        LOG((LOG_ERROR, "请求错误：ret=%d,msg=%s", reader.retcode, reader.retmsg.str));
        ret = SVC_FAILED;
    }
    else
    {
        syspara_clear_feerate();
        syspara_feerate_t feerate;
        while (!sw_message_fetch_row(&reader))
        {
            memset(&feerate, 0, sizeof feerate);
            sw_message_get_row_int(&reader, "feetype", &(feerate.feetype));
            sw_message_get_row_int(&reader, "dpsrate", &(feerate.feerate));
            feerate.feeflag = 0;
            int8 rc = syspara_add_feerate(&feerate);
            if (rc != DB_OK)
            {
                LOG((LOG_ERROR, "保存费率参数错误，%s", emsg));
                disp_msg("保存费率参数错误，", 10);
            }
        }
        // 返回批次号，操作员号
        ret = SVC_OK;
    }
    sw_message_free(&reader);
    sw_message_free(&writer);
    return ret;
}
// }}}
//{{{ allowance functions
int svc_query_allowance(allowance_info_t* allowance)
{
    const char* colnames[] =
    {
        "cardphyid",
        "cardno",
        "cardbefbal",
        "dpscnt"
    };
    sw_message_t reader, writer;
    memset(&reader, 0, sizeof reader);
    memset(&writer, 0, sizeof writer);
    sw_message_reset(&reader);
    sw_message_reset(&writer);
    sw_message_set_attr_int(&writer, "termid", p16pos.termid);
    sw_message_set_attr(&writer, "checknum", p16pos.check_num);
    sw_message_col_define(&writer, colnames, sizeof(colnames) / sizeof(colnames[0]));
    writer.funcno = 950075;

    char cardphyid[33];
    memset(cardphyid, 0, sizeof cardphyid);
    encode_hex(p16card.cardphyid, 4, cardphyid);
    sw_message_set_row(&writer, "cardphyid", cardphyid);
    sw_message_set_row_int(&writer, "cardno", p16card.cardno);
    sw_message_set_row_int(&writer, "cardbefbal", p16card.cardbefbal);
    sw_message_set_row_int(&writer, "dpscnt", p16card.dpscnt);
    sw_message_add_row(&writer);

    // disp_message(disp, "等待下载充值参数", svc_timeout + 1);
    call_context_t ctx;
    memset(&ctx, 0, sizeof ctx);
    ctx.title = "等待查询补助信息";
    ctx.request = &writer;
    ctx.response = &reader;
    ctx.api_call = ECARD_SVC_CALL;
    int ret;
    if ((ret = thread_svc_callsvr(&ctx, svc_timeout)))
    {
        LOG((LOG_ERROR, "请求后台错误，%s", ctx.emsg));
        disp_msg("请求后台错误", 10);
    }
    else if (reader.retcode != 0)
    {
        LOG((LOG_ERROR, "请求错误：ret=%d,msg=%s", reader.retcode, reader.retmsg.str));
        ret = SVC_FAILED;
    }
    else
    {
        int rowcount = 0;
        sw_message_get_attr_int(&reader, "rowcnt", &rowcount);
        if (rowcount > 1000)
        {
            LOG((LOG_ERROR, "补助明细数据太多，无法领取"));
            ret = SVC_FAILED;
        }
        else
        {
            sw_message_get_attr_int(&reader, "totalamt", &allowance->total_amount);
            sw_message_get_attr_int(&reader, "getamt", &allowance->total_get_amt);
            sw_message_get_attr_int(&reader, "dpsamt", &p16card.dpsamt);
            allowance->detail = (allowance_detail_t*)malloc(sizeof(allowance_info_t) * rowcount);
            allowance->allowance_count = rowcount;
            rowcount = 0;
            while (!sw_message_fetch_row(&reader))
            {
                allowance_detail_t detail;
                memset(&detail, 0, sizeof detail);
                sw_message_get_row_int(&reader, "subsidyno", &(detail.subsidyno));
                sw_message_get_row_int(&reader, "amount", &(detail.amount));
                // SW_MSG_SAFE_GET_ROW(&reader, "remark", detail.desc);
                memcpy(&(allowance->detail[rowcount]), &detail, sizeof detail);
                ++rowcount;
            }
            if (rowcount != allowance->allowance_count)
            {
                LOG((LOG_ERROR, "补助返回数据错误"));
                free(allowance->detail);
                allowance->detail = 0;
                ret = SVC_FAILED;
            }
            else
                ret = SVC_OK;
        }
    }
    sw_message_free(&reader);
    sw_message_free(&writer);
    return ret;
}
//{{{ svc_get_allowance function
int svc_get_allowance(p16_transdtl_t* transdtl, allowance_info_t* allowance, int flag)
{
    const char* colnames[] =
    {
        "subsidyno",
    };
    sw_message_t reader, writer;
    memset(&reader, 0, sizeof reader);
    memset(&writer, 0, sizeof writer);
    sw_message_reset(&reader);
    sw_message_reset(&writer);
    sw_message_set_attr(&writer, "checknum", p16pos.check_num);
    sw_message_set_attr(&writer, "termdate", transdtl->transdate);
    sw_message_set_attr(&writer, "termtime", transdtl->transtime);
    sw_message_set_attr_int(&writer, "termseqno", transdtl->devseqno);
    sw_message_set_attr_int(&writer, "termid", transdtl->termid);
    sw_message_set_attr_int(&writer, "retry", flag);
    if (flag == FLAG_QUERY)
    {
        sw_message_set_attr(&writer, "reqflag", "Q");
    }

    sw_message_set_attr(&writer, "cardphyid", transdtl->cardphyid);
    sw_message_set_attr(&writer, "devphyid", transdtl->devphyid);
    sw_message_set_attr(&writer, "samno", transdtl->termno);
    sw_message_set_attr_int(&writer, "cardno", transdtl->cardno);
    sw_message_set_attr_int(&writer, "cardbefbal", transdtl->cardbefbal);
    sw_message_set_attr_int(&writer, "paycnt", transdtl->paycnt);
    sw_message_set_attr_int(&writer, "dpscnt", transdtl->dpscnt);
    sw_message_set_attr_int(&writer, "amount", transdtl->amount);
    sw_message_set_attr_int(&writer, "dpsamt", transdtl->dpsamt);
    sw_message_set_attr(&writer, "random", transdtl->random);
    sw_message_set_attr(&writer, "mac1", transdtl->mac1);

    sw_message_col_define(&writer, colnames, sizeof(colnames) / sizeof(colnames[0]));
    writer.funcno = 950076;

    int row;
    for(row = 0; row < allowance->allowance_count; ++row)
    {
        sw_message_set_row_int(&writer, "subsidyno", allowance->detail[row].subsidyno);
        sw_message_add_row(&writer);
    }

    // disp_message(disp, "等待下载充值参数", svc_timeout + 1);
    call_context_t ctx;
    memset(&ctx, 0, sizeof ctx);
    ctx.title = "等待领取补助";
    ctx.request = &writer;
    ctx.response = &reader;
    ctx.api_call = ECARD_SVC_CALL;
    int ret;
    if ((ret = thread_svc_callsvr(&ctx, svc_timeout)))
    {
        LOG((LOG_ERROR, "请求后台错误，%s", ctx.emsg));
        disp_msg("请求后台错误", 10);
    }
    else if (reader.retcode != 0)
    {
        LOG((LOG_ERROR, "请求错误：ret=%d,msg=%s", reader.retcode, reader.retmsg.str));
        SAFE_STR_CPY(transdtl->errmsg, reader.retmsg.str);
        transdtl->errcode = reader.retcode;
        ret = SVC_FAILED;
    }
    else if (!sw_message_fetch_row(&reader))
    {
        // 返回批次号，操作员号
        SW_MSG_SAFE_GET_ROW(&reader, "mac2", transdtl->mac2);
        SW_MSG_SAFE_GET_ROW(&reader, "hostdate", transdtl->hostdate);
        SW_MSG_SAFE_GET_ROW(&reader, "hosttime", transdtl->hosttime);
        SW_MSG_SAFE_GET_ROW(&reader, "refno", transdtl->refno);
        ret = SVC_OK;
    }
    else
    {
        disp_msg("系统充值返回错误", 10);
        ret = SVC_FAILED;
    }
 
    sw_message_free(&reader);
    sw_message_free(&writer);
    return ret;
}
//}}}
//{{{ svc_allowance_machine_login function
int svc_allowance_machine_login()
{
    const char* colnames[] =
    {
        "reqtype",
        "devphyid",
        "termno",
    };
    sw_message_t reader, writer;
    memset(&reader, 0, sizeof reader);
    memset(&writer, 0, sizeof writer);
    sw_message_reset(&reader);
    sw_message_reset(&writer);
    sw_message_col_define(&writer, colnames, sizeof(colnames) / sizeof(colnames[0]));
    writer.funcno = 850003;

    char samno[13];
    memset(samno, 0, sizeof samno);
    sw_message_set_row(&writer, "reqtype", "ATM;");
    sw_message_set_row(&writer, "devphyid", p16pos.devphyid);
    encode_hex(p16pos.termno, 6, samno);
    sw_message_set_row(&writer, "termno", samno);
    sw_message_add_row(&writer);

    call_context_t ctx;
    memset(&ctx, 0, sizeof ctx);
    ctx.title = "等待终端签到";
    ctx.request = &writer;
    ctx.response = &reader;
    ctx.api_call = ECARD_SVC_CALL;
    int ret;
    p16pos.login_flag = 0;
    if ((ret = thread_svc_callsvr(&ctx, svc_timeout)))
    {
        LOG((LOG_ERROR, "请求后台错误，[%s]", ctx.emsg));
        disp_msg("请求后台错误", 10);
    }
    else if (reader.retcode != 0)
    {
        LOG((LOG_ERROR, "请求错误：ret=%d,msg=%s", reader.retcode, reader.retmsg.str));
        char tip[280];
        sprintf(tip, "签到失败,%d %s", reader.retcode , reader.retmsg.str);
        disp_msg(tip, 10);
        ret = SVC_FAILED;
    }
    else if (!sw_message_fetch_row(&reader))
    {
        // 返回批次号，操作员号
        p16pos.operid = ALLOWANCE_OPER_ID;
        sw_message_get_row_int(&reader, "termid", &(p16pos.termid));
        sw_message_get_row_int(&reader, "termseqno", &(p16pos.termseqno));
        SW_MSG_SAFE_GET_ROW(&reader, "checknum", p16pos.check_num);
        /* sw_message_get_row_int(&reader, "cfgverno", cfgverno); */
        p16pos.login_time_tick = get_time_tick();
        LOG((LOG_DEBUG, "签到成功,termid=%d,seqno=%d", p16pos.termid, p16pos.termseqno));
        ret = SVC_OK;
        p16pos.login_flag = 1;
    }
    else
    {
        disp_msg("系统返回错误", 10);
        ret = SVC_FAILED;
    }
    sw_message_free(&writer);
    sw_message_free(&reader);
    return ret;
}
//}}}
//}}}
// vim:tw=80:fdm=marker

