/*
*Copyright@ 2009 by shenzhen xiaopeng
*ver		:	0.01
*ahthor		:   xp
*data		: 	2009-04-07
*function	:	ad.h
*/
	
#ifndef __OTHER_H
#define	__OTHER_H

#define	 BATT_CHANEL 	0
#define   uchar  unsigned char
#define   uint   unsigned int

enum adcmd_ioctl 
{
	SEL_CHNEL ,
} ;	

//int32	ad_init(void) ;
//void 	ad_close(void) ;
uint8	battey_vol_get(void) ;


char  fun_hextoascii_byte(uchar hex_data);				//ʮ����������ת��ΪASCII��
uchar fun_asciitohex_byte(char ascii_data);				//ASCII��ת��Ϊʮ����������
void  fun_bcd_ascii(uchar *Bcd, uchar *Ascii, uint len);	//��BCD��ת��ΪASCII����
void  fun_ascii_bcd(uchar *Ascii, uchar *Bcd, uint len);	//��ASCII��ת��ΪBCD����
void  fun_clear_buff(uchar *buff, uint bufflen);			//��ջ���������
uint  fun_find_byte(uchar *buff, uint buff_len, uchar findbyte);//�������м��ָ������
uint  fun_find_nbyte(uchar *str1, uint len1, uchar *str2, uint len2);	//
uint  fun_bytetobit_array(uchar *Indata, uint IndataLen, uchar *Outdata, uchar BitNum);
uchar fun_exp2_n(uchar en);
uchar fun_inversion_bit(uchar datain);

	
#endif	
