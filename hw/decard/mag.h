//--------------------------------------------------------------//
//							磁条卡参数定义						//
//--------------------------------------------------------------//
#define MAX_TK_BUF	    	88			// 定義每軌最大資料量(單位Byte)
#define MAX_DATA_BUF    	130         // 定义转换后最大数据量

enum mega_ioctl_cmd
{
	MAGTEK_STATUS ,
	MAGTEK_SERCH ,
	MAGTEK_CANAEL ,	
} ;	

enum mage_status						// 状态
{
	MAGTEK_IDL ,	
	MAGTEK_SRCH ,
	MMAGTEK_ERR ,		
	MAGTEK_END ,	
} ;
	
typedef struct
{
    unsigned char 		len;			            // 数据長度
    unsigned char 		buf[MAX_TK_BUF];	        // 数据暫存區
}MSR_DCB;
	
typedef struct
{
    unsigned char 		len;			            // 数据長度
    unsigned char 		buf[MAX_DATA_BUF];	    	// 数据暫存區
}MSR_ASC_DAT;
	
	
typedef struct
{
	unsigned char		ver[16] ;		//硬件版本	
	MSR_DCB 			tk_buf[6];		//原始数据
	MSR_ASC_DAT		 	track[3];		//解码数据
}magtek_dst ;

	