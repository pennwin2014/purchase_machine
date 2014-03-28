#ifndef _SW_P16_DEPOSIT_LOG_
#define _SW_P16_DEPOSIT_LOG_


#define LOG_LEVEL_BEGIN 1
#define LOG_DEBUG 1
#define LOG_INFO 2
#define LOG_ERROR 3
#define LOG_NOTICE 4

/**
 * @brief 输出错误日志，并退出程序
 * @param errcode - 程序退出码
 * @param msg - 日志信息
 */
void error_exit(int errcode,const char* msg);

/**
 * @brief 错误日志函数
 * @param level - 日志级别 DEBUG, INFO, ERROR, NOTICE
 * @param fmt - 格式
 */
void p16log(int level,const char* fmt,...);

/**
 * @brief - hex格式输出日志
 * @param level - 日志级别
 * @param data - 二进制数据
 * @param len - 数据长度
 */
void p16dump_hex(int level,unsigned char *data,unsigned int len);

#ifdef DEBUG_LOG
#define LOG(x) do{ p16log x; }while(0)
#else
#define LOG(x) do{ ; }while(0)
#endif


#endif // _SW_P16_DEPOSIT_LOG_
