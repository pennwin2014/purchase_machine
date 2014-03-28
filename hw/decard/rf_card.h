/*
*Copyright@ 2008 by shenzhen xiaopeng
*ver		:	0.01
*ahthor		:   xp
*data		: 	2009-03-30
*function	:	card_interface.h
*/
#ifndef 	__CARD_INTERFACE__H
#define		__CARD_INTERFACE__H

enum RC531_IOCTL_COMM
{
	CONFIG_CARD_TYPE,
	EE_READ , 
	CLOSE_RF ,
	OPEN_RF ,
	DC_HALT ,	
	
	TYPEA_REQUST,
	TYPEA_ANTICOLL,
	TYPEA_SECLECT,
	TYPEA_PRO_RESET,
	TYPEA_CARD ,
		
	TYPEB_REQUST,
	TYPEB_ATTRIB,
		
	M1_LOAD_KEY ,
	M1_AUTH_KEY ,
	M1_AUTHOR_PASS ,
	M1_INIT_VAL ,
	M1_READ_VAL ,
	M1_INCREMENT ,
	M1_DECREMENT ,
	M1_RESTORE ,
	M1_WR_RD ,
	M1_EX_MOD,
	
	WR_RD_TIM ,
	DC_SELECT ,
	DC_ANTICOL ,
	
} ;	

typedef	struct
{
	uint8			cmd ;
	uint8			mod ;
	uint8			addr ;
	uint32			val ;
	uint8			len ;		
	int8			buf[16] ;
	
}m1_cmd_dst ;


typedef	struct
{
	uint16			len ;
	uint8			buf[300] ;
	
}tyab_rdwr_dst ;



// ioctl 命令列表
enum Comm_ioctl_List
{
	COMM_RST ,
	COMM_CLOLD_RST ,
	COMM_CAR_DECT ,
	COMM_WR_RD ,
	SEL_CHANEL ,
	SET_CARD_VOL ,
	SET_CARD_FRE ,
	N6001_ID_GET ,
	SET_RTU_MOD ,
	SET_PPS ,
} ;

typedef	struct
{
	unsigned char	chanl ;
	unsigned char	mod ;	
	unsigned char	para ;
	unsigned char 	para1 ;
	unsigned char 	len ;
	unsigned char 	buf[32] ;
}ioctl_cmd1 ;	
		
struct wr_dst
{
	unsigned char 	wbuf[300];
	unsigned short 	w_len ;
			
	unsigned char	rbuf[300] ;
	unsigned short 	r_len ;
	
	
} ;

int dc_config_card(int fd,unsigned char cardtype);
int dc_reset(int fd,int offon);
int dc_pro_reset(int fd,unsigned char *rlen,unsigned char* buff);
int dc_pro_command(int fd, unsigned int slen,unsigned char *sdata,unsigned int *rlen,unsigned char* rbuf,unsigned char timeout);

int32	dc_init_rf(void) ;
void 	dc_exit_rf(int fd) ;
int32 dc_config_shgj(int fd,int cardpara);
int32 	rf531_id_get(int fd,uint8 *ptr) ;
int32 	dc_request(int fd,uint8 _Mode , uint8 *TagType) ;
int32 	dc_anticoll(int fd,uint8 _Bcnt , uint32 *_Snr) ;
int32 	dc_select(int fd,uint32 _Snr , uint8 *_Size) ;
int32 	dc_card(int fd,uint8 _Mode , uint32 *_Snr) ;

int32	dc_anticoll_ioctln(int fd,uint8 mod ,uint32 *_Snr );
int32	dc_select_ioctln(int fd,uint8 mod ,uint32 _Snr , uint8 *_Size);

int32 	dc_authentication_pass(int fd,uint8	_Mode , uint8	sectorno, uint8	*password) ;
int     dc_authentication_passaddr(int icdev,unsigned char _Mode,unsigned char addr,unsigned char *password);

int32 	dc_readval(int fd,uint8	_Adr , uint32	*_Value) ;
int32 	dc_decrement(int fd,uint8	_Adr , uint32	_Value) ;
int     dc_increment(int fd,unsigned char _Adr,unsigned long _Value);
int32 	dc_initval(int fd,uint8	_Adr , uint32	*_Value) ;
int32   dc_restore(int fd,uint8	_Adr1 , uint8	_Adr2);
int dc_read(int fd,unsigned char _Adr,unsigned char *_Data);
int dc_write(int fd,unsigned char _Adr,unsigned char *_Data);

int dc_config_card(int fd,unsigned char cardtype);
//短转长.length为短的长度
int hex_a(unsigned char *hex,unsigned char *a,int length);
//长转短 ，len为长的长度
int a_hex(unsigned char *a,unsigned char *hex,int len);

int dc_reset(int fd,int offon);
int dc_pro_reset(int fd,unsigned char *rlen,unsigned char* buff);
int dc_pro_reset_hex(int fd,unsigned char *rlen, char *receive_data);

int dc_pro_command(int fd, unsigned int slen,unsigned char *sdata,unsigned int *rlen,unsigned char* rbuf,unsigned char timeout);
int dc_pro_command_hex(int fd,unsigned int slen,
									char * sendbuffer,unsigned int *rlen,
									char * databuffer,unsigned char timeout);
																	 
int dc_request_b(int fd,unsigned char _Mode,unsigned char AFI, 
unsigned char N, unsigned char *ATQB);
int dc_attrib(int fd,unsigned char *PUPI, unsigned char CID);
int dc_card_b(int fd,unsigned char *rbuf);
int dc_pro_commandsource(int fd,unsigned int slen,unsigned char * sendbuffer,unsigned int *rlen,unsigned char * databuffer,unsigned char timeout);
int dc_pro_commandsourcehex(int fd,unsigned int slen, char * sendbuffer,unsigned int *rlen, char * databuffer,unsigned char timeout);
int dc_typeb_timeout(int fd , unsigned char timval);
int dc_card_double(int icdev,unsigned char _Mode,unsigned char *Snr);

//////////////////////////////////
int dc_int_sam();
int dc_exit_sam(int fd);
int dc_setcpu(int fd,unsigned char _Byte);
int dc_setcpupara(int fd,unsigned char cputype,unsigned char cpupro,unsigned char cpuetu);
int dc_cpureset(int fd,unsigned char *rlen,unsigned char *databuffer);
int dc_cpureset_hex(int fd,unsigned char *rlen,char *databuffer);

int dc_cpuapdu(int fd,unsigned int slen,unsigned char * sendbuffer, unsigned int *rlen,unsigned char * databuffer);
int dc_cpuapdu_hex(int fd,unsigned int slen,unsigned char * sendbuffer,unsigned int *rlen, char * databuffer);

int dc_set_pps(int fd,unsigned char cpuetu,int len);							 

//////磁条函数
int maginit();
int magschstatus(int fd);
int magdata(int fd,unsigned char* ver,unsigned char* track1,unsigned char* track2,unsigned char* track3);
int magcancel(int fd);




#endif
