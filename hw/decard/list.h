#ifndef		__LIST_H_
#define		__LIST_H_
	
#define		FRAM_WIDTH		3
	
enum Lst_typ
{
	LST_NOTILE = 0x00 ,
	LST_TILE = 0x01 ,
} ;
		
enum LST_PROPER
{
	LST_STA_STATION ,
	LST_LST_TYPE ,	
	LST_WIDTH ,
	LST_ROW_HIGH ,
	LST_SEL_COLOR ,
	LST_FON_COLOR ,
	LST_BAK_COLOR ,
	LST_FS_COLOR ,	
	LST_FONT_TYPE ,
	LST_ROW_LINE ,
	LST_BAR_TYP ,	
	LST_SEL_MOD ,	
	LST_FKEY_EN ,	
	LST_TILE_COLOR ,	//标题颜色
	LST_K12315_EN ,		
} ;
	
struct list_tab
{	
	uint8				dis[50] ;
	struct list_tab		*left ;
	struct list_tab		*next ;
} ; 
		
	
typedef struct
{
	uint16		x0,y0 ;
	uint16		width ;
	
	uint16		type;
	uint16		sel_mod ;
	uint16		bar_typ ;
	uint16		b_color ;
	uint16		f_color ;
	uint16		s_color ;
	uint16		fs_color ;
	uint16		col_num ;
	uint16		lin_num ;
	uint16		lin_hig ;
	uint16		font_hzty ;
	uint16		font_ascty ;
	uint16		fkey_en ;	
	uint16		k12315_en ;
		
	uint16		cur_sel ;
	uint16		cur_sta ;
	uint16		cur_start ;
	uint16		item_cout ;	
	uint16		t_color ;     //标题背景颜色
			
	struct list_tab		*lptr_head ;
	struct list_tab		*lptr_tile ;
	
}list_dst ;
	
	
list_dst 	*list_init(void) ;
void		list_exit(list_dst 	*ptr) ;
void		list_show(dis_map	*dptr ,list_dst 	*ptr) ;
void		list_item_dis(dis_map	*dptr , list_dst 	*ptr) ;
void		list_add(list_dst 	*ptr, struct list_tab *lptr) ;
void		list_item_head_add(list_dst *ptr , ...);
void		list_item_add(list_dst *ptr , ...) ;
int			list_property_set(list_dst *ptr ,int	comm , ...) ;
void		list_item_next(dis_map	*dptr , list_dst  *ptr) ;
void		list_item_last(dis_map	*dptr , list_dst  *ptr);
int			list_val_get(list_dst *ptr);
void		list_props_set(list_dst  *ptr , proptey *pptr) ;
int32		list_key_manege(dis_map *dptr ,list_dst  *ptr, int32(* funp)(void)) ;
	
#endif

