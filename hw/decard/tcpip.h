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
 * ��������ConnectTcpServer() 
 * ���ܣ� ��Ϣ����������
 * ����ֵ��-1��ʾ����ʧ�ܣ�1��ʾ���ӳɹ�
 * ����˵����Ser_IP ��������ַ Ser_Port �������˿�
 *------------------------------------------------ 
 */
extern int connecttcpserver(unsigned char * Ser_IP,int Ser_Port);
/* -----------------------------------------
 * ��������SendTcpMessage() 
 * ���ܣ� ������Ϣ
 * ����ֵ��-1��ʾ����ʧ�ܣ�1��ʾ���ͳɹ�
 * ����˵����smsglen:��Ϣ���� Msg ��Ϣ���� ,��ʱ�趨 
 * --------------------------------------------
 */ 
extern int sendtcpmessage(int smsglen,unsigned char *msg ,int timeout);
/*----------------------------------------------------------
*��������	:	SendMsg
*����	 	:	�������ʱʹ�õĺ������
*�������	:	
*��������	:
*-----------------------------------------------------------
*/	
/*------------------------------------- 
 * ��������RecvTcpMessage
 * ���ܣ� ������Ϣ
 * ����ֵ��-1��ʾ��ʱ�˳���0 ��ʾ ����ʧ�� else ��ʾ������Ϣ����
 * ����˵����Msg ��Ϣ���� �� timeout ��ʱ����
 * 
 *-------------------------------------
 */ 	
 extern int recvtcpmessage(unsigned char *msg, int timeout);
 extern void tcpclose(void);
 /*
 ���÷���
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

