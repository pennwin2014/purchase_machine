#ifndef		__LABEL_H_
#define		__LABEL_H_
	
#define		CAP_MAX_LEN			32
	
enum DIS_STA
{
	STA_LEFT ,
	STA_RIGHT ,
	STA_CENTER ,
			
};
	
enum DIS_LABMOD
{
	DIS_NORMAL ,
	DIS_BACKCOLOR ,
	DIS_BACKDIS ,
	DIS_ADD ,	
};
	
	
enum LBL_PROPER
{
	LABL_STA_STATION ,
	LABL_STATE ,	
	LABL_WIDTH ,
	LABL_FON_COLOR ,
	LABL_BAK_COLOR ,
	LABL_FONT_TYPE ,	
	LABL_DIS_MOD ,
	LABL_DIS_CAPTION ,
} ;

typedef	struct
{
	uint16		x,y ;
	uint16		width,height ;	
	
	uint16		dis_sta ;
	uint16		dis_mod ;
	
	uint16		f_color ;
	uint16		b_color ;	
			
	uint16		font_hzty ;
	uint16		font_ascty ;
	
	uint8		capaction[CAP_MAX_LEN] ;

}label_dst ;

label_dst 	*label_init(void);
void		label_dis(dis_map	*dptr ,label_dst 	*ptr) ;
void		label_dis_bak(dis_map	*dptr ,label_dst 	*ptr) ;
int			label_property_set(label_dst *ptr ,int	comm , ...) ;
void		label_destory(label_dst 	*ptr) ;
void		label_props_set(label_dst *ptr , proptey *pptr) ;

#endif

