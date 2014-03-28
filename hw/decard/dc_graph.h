/*
*Copyright@ 2008 by shenzhen xiaopeng
*ver		:	0.01
*ahthor		:   xp
*data		: 	2008-11-18
*function	:	dc_graph.h
*/
#ifndef 	__DC_GTAPH_H
#define		__DC_GTAPH_H

int32 line(dis_map * ptr , int32 x0 , int32 y0 , int32 x1 , int32 y1 ,uint32 color) ;
int32 dot_line(dis_map * ptr , int32 x0 , int32 y0 , int32 x1 , int32 y1 ,uint32 color) ;
int32 rectangle(dis_map * ptr ,int32 x0 , int32 y0 , int32 x1 , 
						int32 y1 ,uint32 color);
int32 rectangle_circle(dis_map * ptr ,int32 x0 , int32 y0 , int32 x1 , 
						int32 y1 ,uint32 color) ;						
int32 triangle(dis_map * ptr ,int32 x0 , int32 y0 , int32 x1 , int32 y1 ,
				int32 x2 , int32 y2 , uint32 color);


//int32 box(dis_map * ptr , uint16 x0 , uint16 y0 , uint16 x1 , 
//						uint16 y1 ,uint32 color);
int32 box(dis_map * ptr , uint16 x0 , uint16 y0 , uint16 x1 , 
						uint16 y1 ,uint32 color);

int32 box_bak(dis_map * ptr , uint16 x0 , uint16 y0 , uint16 x1 , 
            uint16 y1 );
int32 rectangle_circle_fill(dis_map * ptr ,int32 x0 , int32 y0 , int32 x1 , 
						int32 y1, uint32 color);
void dis_sta_set(dis_map * ptr ,uint16 x , uint16 y) ;
void dis_mod_set(dis_map * ptr ,uint16 x , uint16 y) ;
uint32 disstr_len_get(uint8 *ptr , uint16 hz_mod, uint16 asc_mod) ;
void dis_printf(dis_map * dptr, uint8 *ptr , uint32 f_color) ;
void dot_dat_get(dis_map * dptr, uint16 dat, uint8 *ptr) ;
void dot_tran(dis_map * dptr, zk_dst *zptr, uint8 *ptr , uint32 color) ;
void printf_str(dis_map * dptr, uint16 x, uint16 y, uint8 *ptr , uint32 f_color) ;
void ex_printf_str(dis_map * dptr, uint16 start_x, uint16 start_y, uint16 end_x, uint8 *ptr, uint32 f_color);

#endif
