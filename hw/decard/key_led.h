/*
*Copyright@ 2008 by shenzhen xiaopeng
*ver		:	0.01
*ahthor		:   xp
*data		: 	2008-11-18
*function	:	fs_print_c.c
*/
	
#ifndef __KEY_LED_H_
#define __KEY_LED_H_

#define	P16N		1		
//#define	F1_KEY		64
//#define	F2_KEY		72
//#define	F3_KEY		80
//#define	F4_KEY		88
//	
//#define	ENT_KEY		92
//#define	ESC_KEY		91
//	
//#define	KEY_1		65
//#define	KEY_2		73	
//#define	KEY_3		81
//#define	KEY_4		66	
//#define	KEY_5		74
//#define	KEY_6		82	
//#define	KEY_7		67
//#define	KEY_8		75	
//#define	KEY_9		83
//#define	KEY_0		76
//#define	KEY_DOT		68		//*
//	
//#define	KEY_UP		89
//#define	KEY_DN		90
//	
//#define	KEY_SHIFT	84		//#

#define	F1_KEY		64
#define	F2_KEY		72
#define	F3_KEY		80
#define	F4_KEY		88
	
#define	ENT_KEY		92
#define	ESC_KEY		84
	
#define	KEY_1		65
#define	KEY_2		66	
#define	KEY_3		67
#define	KEY_4		73	
#define	KEY_5		74
#define	KEY_6		75	
#define	KEY_7		81
#define	KEY_8		82	
#define	KEY_9		83
#define	KEY_0		90
#define	KEY_DOT		89		//*
	
#define	KEY_UP		68		//76
#define	KEY_DN		76
	
#define	KEY_SHIFT	91		//#
#define	KEY_12315	96		//#

//enum key_ioctl_cmd
//{
////	LED_UPD ,
////	LED_PARA ,	
////	GET_KEY_NUM ,
////	GET_ID ,
////	BEEP_CMD ,
////	
////	GPRS_RST_PINSET ,
////	HC4051_A_PINSET ,
////	HC4051_B_PINSET ,
////	POWER_PIN_STA,
//
//	LED_UPD ,
//	LED_PARA ,	
//	GET_KEY_NUM ,
//	GET_ID ,
//	BEEP_CMD ,
//	
//	GPRS_RST_PINSET ,
//	HC4051_A_PINSET ,
//	HC4051_B_PINSET ,
//	POWER_PIN_STA,
//	
//	AD8910_MUS_SEL,
//	AD8910_INIT,	
//	
//	GPRS_POWER_SET ,
//
//	HC595_PIN_SET ,	
//	HC595_PIN_CLR ,	
//} ;	
#define	MODEM_RST		8
#define	Hc4052_SELA		12
#define	Hc4052_SELB		13

enum ch452_ioctl_cmd
{
	LED_UPD ,
	LED_PARA ,	
	GET_KEY_NUM ,
	GET_ID ,
	BEEP_CMD ,
	
	GPRS_RST_PINSET ,
	HC4051_A_PINSET ,
	HC4051_B_PINSET ,
	POWER_PIN_STA,
	
	AD8910_MUS_SEL,
	AD8910_INIT,	
	
	GPRS_POWER_SET ,
	
	HC595_PIN_SET ,	
	HC595_PIN_CLR ,
	
	LCD_POWER_SET,
	LCD_DIS_SET,
	LCD_DISTIM_SET,
	LCD_DIS_CON_EN,	
	
	CH452_RE_INIT,
			
} ;		
enum get_key_mod
{
	BLOCK_MOD,
	NOBLOCK_MOD,
	TIM_DLY_MOD,
};

#if	P16N == 1
enum modem_gprs_mod
{
	GPRS_CHL,
	PK_CHL ,
	ERXD3_CHL ,
	MODEM_CHL ,
};
#else	
enum modem_gprs_mod
{
	GPRS_CHL,
	MODEM_CHL ,
	EX1_CHL ,
	EX2_CHL ,
};
#endif

int32 	key_led_init(void) ;	
void 	key_buf_clear(void) ;
void 	beep(uint32 dtim) ;
int32 	key_led_init(void) ;
void 	key_led_close(void) ;
int32 	led_updat(uint8 *ptr) ;
uint8 	get_led_val(uint8 val) ;
void 	power_pin_set(uint32 val) ;
void 	gprs_pin_rst(uint32 val);
int32 	is_num_key(uint8 key_val) ;
void 	dis_buf_tran(uint8 *ptr , uint8 len) ;
int32 	get_key(uint8 *key_ptr , uint32 mod , uint32 dtim) ;
void 	delay(uint32 dtim) ;
uint8 	key_dig_get(uint8 key_val) ;	
void 	sel_comm_chl(int dtim) ;
int 	power_sta_get(void) ;
int 	net_sta_init(void) ;
int 	net_sta_get(void) ;
int 	lcd_pow_set(int val);
void 	modem_rst_set(int level);	
void 	key452_re_init(void);

#endif 
