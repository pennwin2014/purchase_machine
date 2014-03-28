//--------------------------------------------------------------//
//							��������������						//
//--------------------------------------------------------------//
#define MAX_TK_BUF	    	88			// ���xÿ܉����Y����(��λByte)
#define MAX_DATA_BUF    	130         // ����ת�������������

enum mega_ioctl_cmd
{
	MAGTEK_STATUS ,
	MAGTEK_SERCH ,
	MAGTEK_CANAEL ,	
} ;	

enum mage_status						// ״̬
{
	MAGTEK_IDL ,	
	MAGTEK_SRCH ,
	MMAGTEK_ERR ,		
	MAGTEK_END ,	
} ;
	
typedef struct
{
    unsigned char 		len;			            // �����L��
    unsigned char 		buf[MAX_TK_BUF];	        // ���ݕ���^
}MSR_DCB;
	
typedef struct
{
    unsigned char 		len;			            // �����L��
    unsigned char 		buf[MAX_DATA_BUF];	    	// ���ݕ���^
}MSR_ASC_DAT;
	
	
typedef struct
{
	unsigned char		ver[16] ;		//Ӳ���汾	
	MSR_DCB 			tk_buf[6];		//ԭʼ����
	MSR_ASC_DAT		 	track[3];		//��������
}magtek_dst ;

	