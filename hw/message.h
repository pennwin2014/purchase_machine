#ifndef _SW_MESSAGE_H_
#define _SW_MESSAGE_H_


#include "config.h"
#include "cJSON.h"

typedef struct
{
	char* str;
	uint32 max_len;
	uint32 use_len;
} sw_string_t;



/**
 * @brief - ���� string �ṹ
 * @return - ���ص�ַ��ʧ�ܷ��� NULL
 */
int sw_new_string( sw_string_t* str );


/**
 * @brief - �ͷ� string ����
 * @param str
 */
void sw_free_string( sw_string_t* str );
/**
 * @brief - ��չ string ����Ŀռ䣬���ı��ѱ��������
 * @param str - string ָ��
 * @param ext_size - Ҫ���ӵĳ��� ( �ֽ��� )
 * @return - ���� 0 ��ʾ�ɹ��� -1 ��ʾʧ��
 */
int sw_string_extend( sw_string_t* str, uint32 ext_size );
/**
 * @brief - �������� string ����Ŀռ䣬���ı��ѱ��������
 * @param str - string ָ��
 * @param new_size - string �µĴ�С
 * @return - ���� 0 ��ʾ�ɹ��� -1 ��ʾʧ�ܣ� 1 ��ʾ�µĿռ�С��ԭ�ռ�
 */
int sw_string_new_size( sw_string_t* str, uint32 new_size );

int sw_string_cmp( const sw_string_t* str1, const sw_string_t* str2 );
int sw_string_cmp_n( const sw_string_t* str1, const sw_string_t* str2, uint32 n );
int sw_string_copy( sw_string_t* dest_str, const sw_string_t* src_str );
int sw_string_copy_n( sw_string_t* dest_str, const sw_string_t* src_str, uint32 n );
int sw_string_copy_c( sw_string_t* dest_str, const char* src );
int sw_string_copy_cn( sw_string_t* dest_str, const char* src, uint32 n );
int sw_string_cat( sw_string_t* dest_str, const sw_string_t* str );
int sw_string_cat_n( sw_string_t* dest_str, const sw_string_t* str, uint32 n );
int sw_string_cat_c( sw_string_t* dest_str, const char* str );
int sw_string_cat_cn( sw_string_t* dest_str, const char* str, uint32 n );

void sw_string_finalize(sw_string_t* str);

////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	sw_string_t* name;
	sw_string_t* value;
} sw_message_element_t;

typedef struct
{
	int count;
	sw_message_element_t* head;
} sw_message_elem_array;

typedef struct
{
	// ���ܺ�
	int funcno;
	// ������
	int retcode;
	// ������Ϣ
	sw_string_t retmsg;
	int frozen;
	cJSON* root;
	cJSON* current_row;
	int fetch_rownum;
} sw_message_t;

int sw_message_reset( sw_message_t* msg );
void sw_message_free( sw_message_t* msg);
int sw_message_col_define( sw_message_t* msg, const char* colnames[], int colcount );
int sw_message_serialize( sw_message_t* msg, sw_string_t* str );
int sw_message_unserialize( const sw_string_t* str, sw_message_t* msg );
int sw_message_set_attr( sw_message_t* msg, const char* attr_name, const char* attr_value );
int sw_message_set_attr_int( sw_message_t* msg, const char* attr_name, int attr_value );
int sw_message_set_row( sw_message_t* msg, const char* col_name, const char* col_value );
int sw_message_set_row_int( sw_message_t* msg, const char* col_name, int col_value );
int sw_message_add_row( sw_message_t* msg );
int sw_message_get_attr( sw_message_t* msg, const char* attr_name, char* attr_value, int maxlen );
int sw_message_get_attr_int( sw_message_t* msg, const char* attr_name, int* attr_value );
int sw_message_fetch_row( sw_message_t* msg );
int sw_message_get_row( sw_message_t* msg, const char* col_name, char* col_value, int maxlen );
int sw_message_get_row_int( sw_message_t* msg, const char* col_name, int* col_value );

#define SW_MSG_SAFE_GET_ROW(msg,c,v) do {sw_message_get_row(msg,c,v,sizeof(v)-1);} while(0)

////////////////////////////////////////////////////////////////////////////////
#endif // _SW_MESSAGE_H_
