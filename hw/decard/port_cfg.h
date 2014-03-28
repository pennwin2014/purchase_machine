/*
*Copyright@ 2008 by shenzhen xiaopeng
*ver		:	0.01
*ahthor		:   xp
*data		: 	2008-11-18
*function	:	port_cfg.h
*/

#ifndef __PORT_CFG_H_
#define __PORT_CFG_H_
	
// typedef signed char 		int8;
// typedef unsigned char 		uint8;

// typedef signed short 		int16;
// typedef unsigned short 		uint16;

// typedef signed int 			int32;
// typedef unsigned int 		uint32;

// typedef signed long long 	int64;
// typedef unsigned long long 	uint64;


#ifndef uint8
#define uint8  unsigned char
#endif

#ifndef int8 
#define int8   char
#endif

#ifndef uint16
#define uint16  unsigned short
#endif

#ifndef int16
#define int16  short
#endif

#ifndef uint32
#define uint32 unsigned int
#endif
#ifndef int32
#define int32 int
#endif

#ifndef uint64
#define uint64 unsigned long long
#endif

#ifndef int64
#define int64 signed long long
#endif

#define	LCD_TYPE			TFT_2440_16
//#define	LCD_TYPE		LCD_160_160

#endif
