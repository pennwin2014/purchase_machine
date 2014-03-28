/*
*Copyright@ 2009 by shenzhen xiaopeng
*ver		:	0.01
*ahthor		:   xp
*data		: 	2009-04-07
*function	:	choise.h
*/
	
#ifndef __CHOISE_H
#define	__CHOISE_H
	
enum choice_mod_lit
{
	OK_CHOISE = 0x01 ,
	ESC_CHOISE = 0x02 ,
	RE_CHOISE = 0x04 
};	
	
	
typedef struct
{
	uint8	hind[32] ;
	uint8	mod ;
	uint16	font_hzty ;
	uint16	font_ascty ;
	uint16	x0 ;
	uint16	y0 ;
	uint16	width ;
	uint16	height ;	
	
}choise_dst ;

enum CHO_PROPER
{
	CHO_STA,
	CHO_MOD ,	
	CHO_FON_TYPE ,
	CHO_WIDTH ,
	CHO_HIND 	
} ;


typedef struct
{
	uint8	hind[32] ;
	uint8	mod ;
	uint16	font_hzty ;
	uint16	font_ascty ;
	uint8	sta ;	
	uint8	x0 ;
	uint8	y0 ;
	
}plan_dst ;
	
choise_dst *choise_init(uint8 mod) ;
int32 		choise_dis(dis_map *dptr, choise_dst *chptr) ;
int32		choise_key_manege(dis_map *dptr ,choise_dst *ptr, int32(* funp)(void)) ;
int			choice_property_set(choise_dst *ptr ,int	comm , ...) ;
void   choice_props_set(choise_dst  *ptr , proptey *pptr);
int32		choise_hind_set(choise_dst *ptr, uint8 *cptr) ;
void  		choise_destory(choise_dst	*ptr ) ;

plan_dst 	*plan_init(uint8 mod) ;	
void  		plan_destory(plan_dst *ptr) ;
int32 		plan_dis(dis_map	*dptr , plan_dst *pptr) ;
int32		plan_step_dis(dis_map *dptr, plan_dst *pptr, int32(* funp)(void), uint8 step) ;

#endif

