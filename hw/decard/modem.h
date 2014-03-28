/*
*Copyright@ 2009 by shenzhen xiaopeng
*ver		:	0.01
*ahthor		:   xp
*data		: 	2009-09-01
*function	:	modem.h
*/  
#ifndef		__MODEM__H
#define		__MODEM__H

#define		NORMAL_MOD		0
#define		FASTPOS_MOD		1

#define		CF_FILE			/demo/config/modem_config.txt
#define		FPOS_SET0		"AT&F;%C0\N0;+A8E=,,,0;$F2;+MS=V22;+ES=6,,8;+ESA=0,0,,,1,0;S17=13\r"		
#define		FPOS_SET1		"AT&F\\N0;+MS=B103;S17=64\r"  	//AT&F\N0;+MS=B103;S17=64\n
#define		CHTM_SET		"ATS12=20\r"

#define		DILID_CODE		"ATD#,20\r"
#define		DILID_TIM		8000

extern int modem_dat_send(char *ptr , int len);
extern int modem_dat_rec(char *rptr , int *plen);
extern int comm_init(char *devname, long baud, char csize, char parity, char stopb, short vmin, short vtime);
extern int modem_init(int mod);
extern int modem_connet(char *cn_ptr);
extern int modem_dis_connet(void);
#endif
