/*
*Copyright@ 2008 by shenzhen xiaopeng
*ver		:	0.01
*ahthor		:   xp
*data		: 	2008-11-18
*function	:	ds_mem.h
*/

#ifndef 	__DS_MEM_H
#define		__DS_MEM_H

enum DIS_MOD
{
	BIT_L,
	BIT_V,
	BIT_4,	
	TFT_16 ,
	TFT_16V ,
	TFT_24 ,	
	TFT_24V ,	
	CLOR24 ,
	MOD_EN ,
	
} ;	

enum
{
	ASC_5DZ	 ,
	ASC_8DZ	 ,
	
	ASC_12DZ ,
	ASC_16DZ ,
	ASC_24DZ ,
	ASC_32DZ ,
		
	HZ_12DZ	 ,
	HZ_16DZ	 ,
	HZ_24DZ	 ,	
	HZ_32DZ	 ,
	
	ST_24DZ	 ,	
	ST_32DZ	 ,	
	
	ASC_32BDZ ,	
	
	ZM_END	 ,
		
} ;

typedef struct
{		
	uint16 		mod ;
	uint16		width ;
	uint16		height ;
	uint16		size ;
	uint16		ws ;
	uint8		*ptr ;
			
}zk_dst ;

typedef struct
{
	uint16		max_line ;
	uint16		max_vole ;

	uint16		curr_line ;
	uint16		curr_vole ;
	
	uint16		hz_mod ;
	uint16		asc_mod ;
	
	uint16		dis_mod ;
	uint32		buf_size ;
	
	uint16		fmod ;
	uint16		mmod ;	
	int32 		(*init_mptr)(uint8 ** ptr , uint32 len) ;
	int32 		(*pix_mptr)(uint8 *ptr , uint16 x, uint16 y , uint32 mod) ;	
	int32 		(*ful_mptr)(uint8 *ptr , uint16 x0 , uint16 y0 , uint16 x1 , 
						uint16 y1 ,uint32 color) ;	
	int32 		(*pix_gmptr)(uint8 *ptr ,  uint16 x, uint16 y) ;
		
	uint8		*ptr ;
	uint8		*bak_ptr ;	
	uint8		*bak_up_ptr ;
	
}dis_map ;

typedef struct
{
	uint16		mod ;
	uint16 		fmod ;
	uint16 		mmod ;		
	int32 		(*init_mptr)(uint8 ** ptr , uint32 len) ;	
	int32 		(*pix_mptr)(uint8 *ptr ,  uint16 x, uint16 y , uint32 mod) ;
	int32 		(*ful_mptr)(uint8 *ptr , uint16 x0 , uint16 y0 , uint16 x1 , 
						uint16 y1 ,uint32 color) ;
	int32 		(*pix_gmptr)(uint8 *ptr, uint16 x, uint16 y) ;
		
}mod_dst ;
	
dis_map * ds_init(uint16 lin , uint16 vol , uint16 mod) ;
int32 normem_malloc(uint8 ** ptr , uint32 len) ;
int32 lcd160mem_malloc(uint8 ** ptr , uint32 len);
void ds_destory(dis_map * ptr) ;
int32 ds_print(dis_map * ptr) ;
int32 dis_dot(dis_map * ptr, uint16 x, uint16 y , uint32 mod) ;
int32 dis_color_dot(dis_map * ptr, uint16 x, uint16 y , uint32 col) ;
int32 dis_pix(dis_map * ptr, uint16 x, uint16 y , uint32 col) ;


#endif
