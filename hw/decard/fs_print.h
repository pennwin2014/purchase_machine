/*
*Copyright@ 2008 by shenzhen xiaopeng
*ver		:	0.01
*ahthor		:   xp
*data		: 	2008-11-18
*function	:	fs_print.h
*/
#ifndef	__FS_PRINT__H
#define	__FS_PRINT__H
	
enum pri_sta
{
	PRIN_IDLE ,
	PRIN_NOPAPER ,
	PRIN_END ,	
	PRIN_STA ,		
};	
	
enum fsprint_ioctl
{
	CANSEL_PRINT,
	HALT_PRINT,
	RESUME_PRINT,
	PRINT_STATUS ,
	FS_SLEF_TEST ,	
	FS_MOD_SET ,
	FS_TIMES_SET ,		
};	
	
struct fsprint_sta
{
	uint16			status ;
	uint32			curr ;	
	uint32			totel ;
} ;
//#define	DEBUG_EN			1
	
#define	FS_DOT_NUM			384
	
int32	fs_print_init(void) ;
int32	fs_print_selftest(void) ;
int32	fs_print_dat(uint8 *ptr , uint32 lenth) ;
int32 	fs_print_pix(uint8 * ptr, uint16 x, uint16 y , uint32 mod) ;
int32	fs_print_close(void) ;
int32 	fs_print_status(uint8 *step) ;
void 	fs_conmod_set(int32 mod);
void 	fs_contim_set(int32 mod);
	
#endif
