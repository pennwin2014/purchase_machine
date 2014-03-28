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


/******************************************�ڲ����ú���************************************************/



/*****************************************************************************************************/ 


/*****************************************�ⲿ���ú���**************************************************/
extern uchar cdma_set_baud(uint baudsel);   //���õ�ǰ�豸�Ĵ���ͨ������Ϊ115200
extern int cdma_init_port(char *devname, long baud, char csize, char parity, char stopb, short vmin, short vtime);

extern uchar cdma_test(void);//����ģ��
extern uchar cdma_close_return(void);   //�رջ���
extern uchar cdma_check_signal(uchar *signal_buff, uchar *signal_len);  //��鵱�ص������ź�ǿ�ȣ�31���0��С
extern uchar cdma_check_sim(uchar *pin_buff, ushort *pin_len);
extern uchar cdma_build_ppp(uchar *status);

extern uchar cdma_set_user(uchar *user,uchar userlen,uchar *passward,uchar passwardlen);  //���� PPP �û����Ϳ�����������
extern uchar cdma_closed_ppp(void); //DTU �ر�PPP ����

//extern uchar cdma_build_tcpip(uchar *ip, uchar iplen, uchar *port, uchar portlen);  //��һ��tcp����
extern uchar cdma_build_tcpip(uchar *ip, uchar iplen, uchar *port, uchar portlen, uchar *localport,uchar localportlen);

extern uchar cdma_check_tcp(uchar *data_buff,uchar *data_len);  //��ѯ��ǰTCP����״̬
extern uchar cdma_close_tcpip(void);  //�ر�TCP����

extern  uchar cdma_send_tcpdata(uchar *data,ushort data_len);  //DTU ����TCP ����


extern uchar cdma_check_sim(uchar *data_buff, ushort *data_len); //��ѯPIN״̬

extern uchar cdma_tcp_recv(uchar *data_buff, ushort *data_len); //��������
extern void  cdmaclose(int fd);
extern uchar cdma_rece_data(uint timout);
#endif
/*******************************************************************************************************
**                            End Of File
********************************************************************************************************/
