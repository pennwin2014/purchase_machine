/****************************************Copyright (c)****************************************************
**                         SHENZHEN DECARD TECHNOLOGIES Co.,LTD.                               
**                                     
**                               http://www.decard.com
**
**--------------File Info---------------------------------------------------------------------------------
** Create by:			roc
** Create date:			2009-03-09
** Version:             1.0
** Descriptions:		
*********************************************************************************************************/
#ifndef 	_GPRS
#define 	_GPRS

#define uchar  unsigned char
#define uint unsigned int  

/***********************************�ӿڶ���*************************************************************/
#define		GPRS_PORT_SELECT		(1<<25)			//�ߵ�ƽ��GPRSģ�飬�͵�ƽ���û��˿�


/******************************************������******************************************************/
#define		GERR_				0		//�û���������ע�ᱻ�����ܾ�������APN ���󣬿�Ƿ�ѣ�����֧��GPRSҵ��ȡ�
#define		GERR_NOLOGIN		1		//û�е�½����
#define		GERR_LINKEXIST		2		//��ǰ�Ѿ������Ӵ���
#define		GERR_PARA			3		//����������Ϸ�
#define		GERR_LINK			4		//TCP ���ӱ��Է��ܾ�
#define		GERR_LINKOVERTIME	5		//TCP ���ӳ�ʱ������IP �Ͷ˿ڲ���ȷ
#define		GERR_MLINK			6		//��ǰ�Ѿ���������������
#define		GERR_SLINK			7		//��ǰ�Ѿ���������������
#define		GERR_LINKREFUSE		8		//��ǰ���������Ӵ��ڣ������޸�link_flag�����IOMODE��
#define		GERR_TCPNOLINK		9		//��ǰû��TCP ��UDP ���Ӵ���(���IPSEND)
#define		GERR_RECEIVEFULL	11		//���ջ�����
#define		GERR_RECEIVEEMPTY	12		//���ջ���������
#define		GERR_OVERLOAD		13		//������Ч���ݳ���1024 �ֽ�
#define		GERR_EVENCHAR		14		//���û�ģʽ�£��ֽ���Ӧ��Ϊż������
#define		GERR_ILLEGALCHAR	15		//�û�ģʽ��������ַǷ��ַ�
#define		GERR_SERIALERR		16		//����Ų�����
#define		GERR_DELETTYPE		17		//�Ƿ���ɾ������
#define		GERR_OTHER			18		//��������
#define		GERR_LINKFAIL		20		//����ʧ��
#define		GERR_PDPNOACTIVE	21		//��ǰû�����PDP ����
#define		GERR_TCPACTIVE		22		//��ǰ%ETCPIP ���ڼ���״̬������ʹ�ø�����
#define		GERR_DOMAINNOBEING	23		//����������
#define		GERR_DOMAINOVERTIME	24		//����������ʱ
#define		GERR_DOMAINUNKNOWN	25		//��������δ֪����
#define		GERR_SERVERTURN		26		//�������ѿ���
#define		GERR_SENDFULL		27		//TCP ���ͻ�������


#define		GPRSBUFFLEN			2048
#define		OKSTRLEN			2
#define		CSQSTRLEN			3
#define		ERRORSTRLEN			5
#define		CONNECTSTRLEN		7
#define		VERSIONLEN			10


extern	uchar gprs_error;						//ģ�����״̬��
extern	uchar gprs_buff[GPRSBUFFLEN];     		//GPRS����/�������ݻ�����
/*****************************************�ⲿ���ú���***************************************************/
extern  int gprs_init_port(char *devname, long baud, char csize, char parity, char stopb, short vmin, short vtime);			//
extern	uchar gprs_set_baud(uint baudsel);		//�����ʳ�ʼ��
extern	uchar gprs_test(void);					//ATָ�����
extern	uchar gprs_close_return(uchar mode);	//���Է�ʽ����/�ر�
extern	uchar gprs_sleep(uchar mode);			//����ģ���Ƿ�����
extern	uchar gprs_check_signal(uchar *signal);	//����ź�ǿ��
extern	void  gprs_close(void);					//�ر�GPRSģ��
extern	uchar gprs_check_sim(void);
extern	uchar gprs_register(uchar mode);
extern	uchar gprs_close_tcp(void);				//�ر�����
extern	uchar gprs_init_apn(void);				//��ʼ��PDP�������
extern	uchar gprs_init_tcpip(void);			//��ʼ��TCPIP
extern	uchar gprs_set_function(uchar mode);	//��������
extern	uchar gprs_querfy_data(uint *data_len);	//��ѯ������ʣ������
extern	uchar gprs_set_datadeletmode(uchar mode);//����ɾ��ģʽ����
extern	uchar gprs_set_datamode(uchar mode);	//��������ģʽ
extern	uchar gprs_check_version(uchar *hversion, uchar *sversion);	//���ģ��汾��
extern	uchar gprs_open_tcp(uchar *ip, uchar iplen, uchar *port, uchar portlen);//��һ��tcp����
extern	uchar gprs_send_tcpdata(char *send_buff, uint send_len);	//ͨ��TCPIPģʽ����һ����������
extern	uchar gprs_receive_tcpdata(uchar *data_buff, uint *data_len);//���ջ���������
extern  void  comclose();
extern  uchar gprs_call_on(uchar *call);//�绰����
extern  uchar gprs_call_off(void);//���ŹҶ�
extern  uchar gprs_buff_clean(uchar mode);//���������
extern  uchar gprs_inittcpip(uchar *user,uchar *pword);
extern  uchar gprs_initapn(char *apn_buff);
extern  uchar gprs_open_check(void);//��ѯ�Ƿ�����
extern  uchar gprs_buff_check(uint *num);//��ѯ����������

void modem_gprs_sem_init(void);
void modem_gprs_sem_end(void);
int gprs_sel_sem_trywait(void);
void gprs_sel_sem_wait(void);
void modem_sel_sem_wait(void);

#endif


/*******************************************************************************************************
**                            End Of File
********************************************************************************************************/

