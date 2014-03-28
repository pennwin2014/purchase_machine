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

/***********************************接口定义*************************************************************/
#define		GPRS_PORT_SELECT		(1<<25)			//高电平接GPRS模块，低电平接用户端口


/******************************************错误定义******************************************************/
#define		GERR_				0		//用户名和密码注册被网络侧拒绝，或者APN 错误，卡欠费，卡不支持GPRS业务等。
#define		GERR_NOLOGIN		1		//没有登陆网络
#define		GERR_LINKEXIST		2		//当前已经有链接存在
#define		GERR_PARA			3		//输入参数不合法
#define		GERR_LINK			4		//TCP 连接被对方拒绝
#define		GERR_LINKOVERTIME	5		//TCP 连接超时，可能IP 和端口不正确
#define		GERR_MLINK			6		//当前已经启动多连接命令
#define		GERR_SLINK			7		//当前已经启动单连接命令
#define		GERR_LINKREFUSE		8		//当前处于有连接存在，不能修改link_flag（针对IOMODE）
#define		GERR_TCPNOLINK		9		//当前没有TCP 或UDP 连接存在(针对IPSEND)
#define		GERR_RECEIVEFULL	11		//接收缓存满
#define		GERR_RECEIVEEMPTY	12		//接收缓存无数据
#define		GERR_OVERLOAD		13		//输入有效数据超过1024 字节
#define		GERR_EVENCHAR		14		//在用户模式下，字节数应该为偶数个数
#define		GERR_ILLEGALCHAR	15		//用户模式下输入出现非法字符
#define		GERR_SERIALERR		16		//该序号不存在
#define		GERR_DELETTYPE		17		//非法的删除类型
#define		GERR_OTHER			18		//其他错误
#define		GERR_LINKFAIL		20		//连接失败
#define		GERR_PDPNOACTIVE	21		//当前没有完成PDP 激活
#define		GERR_TCPACTIVE		22		//当前%ETCPIP 处于激活状态，不能使用该命令
#define		GERR_DOMAINNOBEING	23		//域名不存在
#define		GERR_DOMAINOVERTIME	24		//域名解析超时
#define		GERR_DOMAINUNKNOWN	25		//域名解析未知错误
#define		GERR_SERVERTURN		26		//服务器已开启
#define		GERR_SENDFULL		27		//TCP 发送缓冲区满


#define		GPRSBUFFLEN			2048
#define		OKSTRLEN			2
#define		CSQSTRLEN			3
#define		ERRORSTRLEN			5
#define		CONNECTSTRLEN		7
#define		VERSIONLEN			10


extern	uchar gprs_error;						//模块错误状态字
extern	uchar gprs_buff[GPRSBUFFLEN];     		//GPRS发送/接收数据缓冲区
/*****************************************外部调用函数***************************************************/
extern  int gprs_init_port(char *devname, long baud, char csize, char parity, char stopb, short vmin, short vtime);			//
extern	uchar gprs_set_baud(uint baudsel);		//波特率初始化
extern	uchar gprs_test(void);					//AT指令测试
extern	uchar gprs_close_return(uchar mode);	//回显方式开启/关闭
extern	uchar gprs_sleep(uchar mode);			//设置模块是否休眠
extern	uchar gprs_check_signal(uchar *signal);	//检测信号强度
extern	void  gprs_close(void);					//关闭GPRS模块
extern	uchar gprs_check_sim(void);
extern	uchar gprs_register(uchar mode);
extern	uchar gprs_close_tcp(void);				//关闭链接
extern	uchar gprs_init_apn(void);				//初始化PDP激活参数
extern	uchar gprs_init_tcpip(void);			//初始化TCPIP
extern	uchar gprs_set_function(uchar mode);	//功能设置
extern	uchar gprs_querfy_data(uint *data_len);	//查询缓冲区剩余数据
extern	uchar gprs_set_datadeletmode(uchar mode);//数据删除模式设置
extern	uchar gprs_set_datamode(uchar mode);	//设置数据模式
extern	uchar gprs_check_version(uchar *hversion, uchar *sversion);	//检测模块版本号
extern	uchar gprs_open_tcp(uchar *ip, uchar iplen, uchar *port, uchar portlen);//打开一条tcp连接
extern	uchar gprs_send_tcpdata(char *send_buff, uint send_len);	//通过TCPIP模式发送一定长度数据
extern	uchar gprs_receive_tcpdata(uchar *data_buff, uint *data_len);//接收缓冲区数据
extern  void  comclose();
extern  uchar gprs_call_on(uchar *call);//电话拨号
extern  uchar gprs_call_off(void);//拨号挂断
extern  uchar gprs_buff_clean(uchar mode);//清除缓冲区
extern  uchar gprs_inittcpip(uchar *user,uchar *pword);
extern  uchar gprs_initapn(char *apn_buff);
extern  uchar gprs_open_check(void);//查询是否在线
extern  uchar gprs_buff_check(uint *num);//查询缓冲区数据

void modem_gprs_sem_init(void);
void modem_gprs_sem_end(void);
int gprs_sel_sem_trywait(void);
void gprs_sel_sem_wait(void);
void modem_sel_sem_wait(void);

#endif


/*******************************************************************************************************
**                            End Of File
********************************************************************************************************/

