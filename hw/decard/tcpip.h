#ifdef _WINDOWS_
**                         SHENZHEN DECARD TECHNOLOGIES Co.,LTD.                               
**                                     
**                               http://www.decard.com
**
**--------------File Info---------------------------------------------------------------------------------
** Create by:			sh
** Create date:			2009-09-30
** Version:             1.0
** Descriptions:		
*********************************************************************************************************/
#include <windows.h> 
#include <winbase.h>
 /* ----------------------------------------------
 * 函数名：ConnectTcpServer() 
 * 功能： 消息服务器连接
 * 返回值：-1表示连接失败，1表示连接成功
 * 参数说明：Ser_IP 服务器地址 Ser_Port 服务器端口
 *------------------------------------------------ 
 */
extern int connecttcpserver(unsigned char * Ser_IP,int Ser_Port);
/* -----------------------------------------
 * 函数名：SendTcpMessage() 
 * 功能： 发送消息
 * 返回值：-1表示发送失败，1表示发送成功
 * 参数说明：smsglen:信息长度 Msg 信息内容 ,超时设定 
 * --------------------------------------------
 */ 
extern int sendtcpmessage(int smsglen,unsigned char *msg ,int timeout);
/*----------------------------------------------------------
*函数名称	:	SendMsg
*功能	 	:	对外调用时使用的函数入口
*参数输出	:	
*参数输入	:
*-----------------------------------------------------------
*/	
/*------------------------------------- 
 * 函数名：RecvTcpMessage
 * 功能： 接收消息
 * 返回值：-1表示超时退出，0 表示 接收失败 else 表示接收信息长度
 * 参数说明：Msg 消息内容 ， timeout 超时设置
 * 
 *-------------------------------------
 */ 	
 extern int recvtcpmessage(unsigned char *msg, int timeout);
 extern void tcpclose(void);
 /*
 调用方法
 if(ConnectTcpServer(SerIP,Port)==-1)			  					
	 	  			return -1;
	 	  	else 
	 	  		{
	 	  			sendLen=strlen(msg);
	 	  			ret=SendTcpMessage(sendLen,msg,2);
	 	  			if (ret==1)
	 	  				{
	 	  					outLen=RecvTcpMessage(outMsg,2);
	 	  					if (outLen!=-1)
	 	  						{
	 	  							return outLen;	 	  							
	 	  						}
	 	  					else
	 	  						{
	 	  							return -1;	
	 	  						}
	 	  					printf("outLen is:%d\n",outLen); 
	 	  				}	 	  		
	 	  		}
	 	  		
 */
#endif 

