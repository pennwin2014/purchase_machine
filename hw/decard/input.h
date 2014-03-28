/*
*Copyright@ 2009 by shenzhen xiaopeng
*ver		:	0.01
*ahthor		:   xp
*data		: 	2009-02-16
*function	:	input.h
*/
#ifndef	__INPUT__H
#define	__INPUT__H
	
typedef struct
{
	uint16		mod ;
	uint16		hint_x0,hint_y0 ;
	uint16		rect_x0,rect_y0 ;
	uint16		rect_height,rect_widith ;
	uint16		cursorx, cursory ;
	uint16		cursor_len ;
	uint16		font_hzty ;
	uint16		font_ascty ;
	uint16		fkey_en ;
	uint16		k12315_en ;		
	uint16		bclor ;	
	uint16		bclor_en ;
	uint16		sta ;
	uint16		num ;
	uint16		max_len ;
	uint16		updn_en ;	 			//增加对UP DN键的处理	
	uint16		bakcor ;
	uint16		bak_mod ;
	uint8		hind[36] ;
	uint8		info[36] ;
		
}input_dst ;
	
enum INPUT_MOD
{
	LITTLE_MONEY_MOD ,			//小金额模式
	FREE_MOD,					//自由输入模式
	PASSWORD_MOD,				//密码输入模式
	YL_MOD,						//银联模式	
	FREE_LMOD,					//左侧开始的自由输入模式
	PASSWORD_LMOD,				//左侧开始的密码输入模式	
} ;	

enum INPUT_PROPER
{
	INPUT_HIN_STA ,
	INPUT_TYPE ,
	INPUT_HIND ,
	INPUT_RET_STA ,
	INPUT_RET_SIZ ,	
	INPUT_HIND_FONT ,
	INPUT_MAX_LEN ,
	INPUT_INFO_SET ,
	INPUT_UODN_SET ,
	INPUT_INFO_ADD ,
	INPUT_FKEY_EN,
	INPUT_BCOR_SET,
	INPUT_BACK_MOD,
	INPUT_K12315_EN,		
} ;
	
input_dst 	*input_init(uint8 mod) ;
int32 		input_dis(dis_map	*dptr , input_dst *ptr) ;
uint32		input_val_get(input_dst *ptr) ;
void 		input_str_get(input_dst *ptr, uint8 *buf) ;
int32 		input_showe(dis_map	*dptr , input_dst *ptr) ;
int32		input_property_set(input_dst *ptr ,int	comm , ...) ;
void		input_props_set(input_dst  *ptr , proptey *pptr) ;
void 		input_destory(input_dst *ptr) ;
int32		input_key_manege(dis_map *dptr ,input_dst *ptr, int32(* funp)(void)) ;
	
#endif	
