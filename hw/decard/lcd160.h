/*
*Copyright@ 2008 by shenzhen xiaopeng
*ver		:	0.01
*ahthor		:   xp
*data		: 	2008-11-18
*function	:	fs_print.h
*/
#ifndef	__LCD160__H
#define	__LCD160__H


//#define	DEBUG_EN			1

#define	LCD_DOT_NUM			160
#define	LCD_REL_COL			81

//typedef signed char 		int8;
//typedef unsigned char 		uint8;
//
//typedef signed short 		int16;
//typedef unsigned short 		uint16;
//
//typedef signed int 			int32;
//typedef unsigned int 		uint32;
//
//typedef signed long long 	int64;
//typedef unsigned long long 	uint64;

enum{
	
	WHITE_COLR 	= 0xffff ,		//255,255,255
	BLACK_COLR 	= 0x0000 ,		//0,0,0
	RED_CLOR 	= 0xf800 , 		//255,0,0
	YELLOW_CLOR = 0xffe0 ,	 	//255,255,0	
	BLUE_CLOR 	= 0x001f ,	 	//0,0,255
	QUEE_CLOR 	= 0x87ff ,	 	//128,255,255	
	GRAY_CLOR 	= 0x8410 ,		//128,128,128
	GRAYL_CLOR 	= 0xc618 ,		//192,192,192
//	GRAYL_CLOR 	= 0xc618 ,		//192,192,192	
		
	} ;
	
enum ioctl_cmd
{
	DIS_BUF ,
	DIS_UPDAT ,
	DIS_CLR ,
	DIS_MMAP ,
} ;	

typedef struct 
{
	uint16		preoty ;
	uint16		para1 ;	
	uint16		para2 ;	
}proptey;

int32	lcd_160_init(void) ;
int32	lcd_160_wr(uint8 *ptr , uint32 lenth) ;
int32	lcd_160_clr(void) ;
int32	lcd_160_upd(void) ;
int32	lcd_160_close(void) ;
int32	lcd_160_pix(uint8 *ptr , uint16 x, uint16 y , uint32 mod) ;
int32	lcd_160_full(uint8 *ptr , uint16 x0 , uint16 y0 , uint16 x1 , 
						uint16 y1 ,uint32 color) ;
extern	uint8		*lcd160_bufptr ;

#endif
