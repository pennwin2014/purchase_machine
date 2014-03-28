/*
*Copyright@ 2009 by shenzhen xiaopeng
*ver		:	0.01
*ahthor		:   xp
*data		: 	2009-02-16
*function	:	picture_list.h
*/
#ifndef		_PIC_LIST_H_
#define		_PIC_LIST_H_
	
#define		HIND_LAB		0
#define		HIND_DWN		1
#define		HIND_LFT		2

#define		HIND_DWN_BAK	10
	
//bmp
struct bmp_filehead
{
	uint8		format[2] ;
	uint32		filesize ;
	uint16		reserved1 ;	
	uint16		reserved2 ;
	uint32		offset ;
		
}__attribute__((packed));
	
struct bmp_fileinfo
{
	uint32 		img_inf_size; 
	uint32 		biWidth; 
	uint32	 	biHeight; 
	uint16 		biPlanes; 
	uint16 		biBitCount; 
	uint32		biCompression; 
	uint32		biSizeImage; 
	uint32 		biXPelsPerMeter; 
	uint32 		biYPelsPerMeter; 
	uint32 		biClrUsed; 
	uint32 		biClrImportant; 
	
}__attribute__((packed));
	
struct bmp_info
{
	struct 		bmp_filehead	head;
	struct 		bmp_fileinfo 	info;
};
	
struct bmp_mod
{
	uint8		path[64];
	uint8		capt[32];
	uint8		mod;
	struct 		bmp_mod	*nptr ;	
} ;	
	
enum pic_lst_comm
{
	BMP_STAT_SET ,
	BMP_SIZE_SET ,
	BMP_INTE_SET ,
	BMP_DMOD_SET ,
	
	BMP_CAPEN_SET ,
	BMP_CAPMOD_SET ,
	BMP_CAPSTA_SET ,
	BMP_CAPSIZ_SET ,
	BMP_CAPFON_SET ,
	
	PIC_BAK_SET ,
	
	HND_MOD_SET ,
	HND_STA_SET ,
	HND_FON_SET ,
	HND_FON_COL_SET ,
	BAK_COL_SET ,
	BAK_COL_MOD ,
	BAK_COL_SIZE ,	
	BAK_FILL_MOD ,	
	BAK_FILL_COL_SET ,		
};
	
struct pic_lst
{	
	uint16	 	x,y ;
	uint16	 	interval ;
	uint16	 	size ;
	uint16	 	high ;
	struct 		bmp_mod	*ptr ;
	
	uint16	 	hind_mod ;
	uint16	 	hind_x, hind_y;
	uint16		hind_hzty, hind_ascty ;
	uint16		hind_hzcor;	
				
	//sel 
	uint16	 	mod ;
	label_dst 	*lab_ptr ;
	uint16	 	cur_sel ;
	uint16		cur_sta ;
	uint16		cur_start ;
	uint16		item_cout ;
	uint16		back_mod ;
	uint16		back_cor;
	uint16		back_cor_mod;
	uint16		back_cor_width;
	uint16		back_cor_height;
	uint16		fill_cor_mod;
	uint16		fill_cor;		
};

struct 	pic_lst *piclist_init(void) ;
void	piclist_destory(struct pic_lst *ptr) ;
int32	piclist_add(struct pic_lst *ptr , uint8 *cptr ,uint8 *cptr1) ;
//void 	hind_lst_init(struct hind_lst *ptr) ;
void	piclist_props_set(struct pic_lst *ptr , char *cptr[] , proptey *pptr) ;
int32	piclist_property_set(struct pic_lst *ptr ,int	comm , ...) ;
int32	piclist_inf_show(dis_map *dptr ,struct pic_lst *ptr , uint16 item , uint16 mod);
int32	piclist_show(dis_map	*dptr ,struct pic_lst *ptr) ;
int32	piclist_next(dis_map *dptr ,struct pic_lst *ptr) ;
int32	piclist_last(dis_map *dptr ,struct pic_lst *ptr) ;
int32	piclist_val_get(struct pic_lst *ptr) ;
int32	piclist_key_manege(dis_map *dptr ,struct pic_lst *ptr, int32(* funp)(void)) ;
int32	piclist_modify(struct pic_lst *ptr , uint8 n , uint8 *cptr ,uint8 *cptr1) ;
int32	bmp_display(dis_map *tstptr, uint16 x, uint16 y , uint8 *fptr) ;
int32	jpg_display(dis_map *tstptr, uint16 x, uint16 y , uint8 *fptr) ;
int32	jpg_stormem(dis_map *tstptr, uint8 *mptr, uint16 x, uint16 y , uint8 *fptr);
int32	piclist_hind_display(dis_map *dptr ,struct pic_lst *ptr);
unsigned short RGB888toRGB565(unsigned char red, unsigned char green, unsigned char blue) ;

#endif 
