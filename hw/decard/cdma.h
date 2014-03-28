/****************************************Copyright (c)****************************************************
**                         SHENZHEN DECARD TECHNOLOGIES Co.,LTD.                               
**                                     
**                               http://www.decard.com
**
**--------------File Info---------------------------------------------------------------------------------
** Create by:			roc
** Create date:			2009-04-12
** Version:             1.0
** Descriptions:		CDMA
*********************************************************************************************************/
#ifndef 	_CDMA
#define 	_CDMA


#define ushort	unsigned short 
#define uchar   unsigned char
#define uint    unsigned int


/******************************************内部调用函数************************************************/



/*****************************************************************************************************/ 


/*****************************************外部调用函数**************************************************/
extern uchar cdma_set_baud(uint baudsel);   //设置当前设备的串口通信速率为115200
extern int cdma_init_port(char *devname, long baud, char csize, char parity, char stopb, short vmin, short vtime);

extern uchar cdma_test(void);//测试模块
extern uchar cdma_close_return(void);   //关闭回显
extern uchar cdma_check_signal(uchar *signal_buff, uchar *signal_len);  //检查当地的网络信号强度，31最大，0最小
extern uchar cdma_check_sim(uchar *pin_buff, ushort *pin_len);
extern uchar cdma_build_ppp(uchar *status);

extern uchar cdma_set_user(uchar *user,uchar userlen,uchar *passward,uchar passwardlen);  //设置 PPP 用户名和口令设置命令
extern uchar cdma_closed_ppp(void); //DTU 关闭PPP 连接

//extern uchar cdma_build_tcpip(uchar *ip, uchar iplen, uchar *port, uchar portlen);  //打开一条tcp连接
extern uchar cdma_build_tcpip(uchar *ip, uchar iplen, uchar *port, uchar portlen, uchar *localport,uchar localportlen);

extern uchar cdma_check_tcp(uchar *data_buff,uchar *data_len);  //查询当前TCP连接状态
extern uchar cdma_close_tcpip(void);  //关闭TCP链接

extern  uchar cdma_send_tcpdata(uchar *data,ushort data_len);  //DTU 发送TCP 数据


extern uchar cdma_check_sim(uchar *data_buff, ushort *data_len); //查询PIN状态

extern uchar cdma_tcp_recv(uchar *data_buff, ushort *data_len); //接收数据
extern void  cdmaclose(int fd);
extern uchar cdma_rece_data(uint timout);
#endif
/*******************************************************************************************************
**                            End Of File
********************************************************************************************************/
