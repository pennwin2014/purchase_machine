#ifndef _SW_P16_DEPOSIT_LOG_
#define _SW_P16_DEPOSIT_LOG_


#define LOG_LEVEL_BEGIN 1
#define LOG_DEBUG 1
#define LOG_INFO 2
#define LOG_ERROR 3
#define LOG_NOTICE 4

/**
 * @brief ���������־�����˳�����
 * @param errcode - �����˳���
 * @param msg - ��־��Ϣ
 */
void error_exit(int errcode,const char* msg);

/**
 * @brief ������־����
 * @param level - ��־���� DEBUG, INFO, ERROR, NOTICE
 * @param fmt - ��ʽ
 */
void p16log(int level,const char* fmt,...);

/**
 * @brief - hex��ʽ�����־
 * @param level - ��־����
 * @param data - ����������
 * @param len - ���ݳ���
 */
void p16dump_hex(int level,unsigned char *data,unsigned int len);

#ifdef DEBUG_LOG
#define LOG(x) do{ p16log x; }while(0)
#else
#define LOG(x) do{ ; }while(0)
#endif


#endif // _SW_P16_DEPOSIT_LOG_
