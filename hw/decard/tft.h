/*
*Copyright@ 2008 by shenzhen xiaopeng
*ver		:	0.01
*ahthor		:   xp
*data		: 	2008-11-18
*function	:	tft.h
*/
#ifndef	__TFT__H
#define	__TFT__H


typedef struct{
	
		int             fd ;
		unsigned char 	*fbmem;
		unsigned int    screensize;
		unsigned long 	fb_mem_offset;
		unsigned int    fb_width;
		unsigned int    fb_height;
		unsigned int    fb_depth;
		unsigned int    x;
		unsigned int    y;	
	
		}fb_str ;

int tft_init(void) ;
int fb_pix(unsigned char *ptr, unsigned int x , unsigned y , unsigned short color) ;
void fb_full(uint8 *ptr , uint16 x0 , uint16 y0 , uint16 x1 , 
						uint16 y1 ,uint32 color);
int fb_gpix(unsigned char *ptr, unsigned int x , unsigned int y) ;						
extern	fb_str	fb_vail ;

#endif
