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
 * @brief - 创建 string 结构
 * @return - 返回地址，失败返回 NULL
 */
int sw_new_string( sw_string_t* str );


/**
 * @brief - 释放 string 对象
 * @param str
 */
void sw_free_string( sw_string_t* str );
/**
 * @brief - 扩展 string 对象的空间，不改变已保存的数据
 * @param str - string 指针
 * @param ext_size - 要增加的长度 ( 字节数 )
 * @return - 返回 0 表示成功， -1 表示失败
 */
int sw_string_extend( sw_string_t* str, uint32 ext_size );
/**
 * @brief - 重新设置 string 对象的空间，不改变已保存的数据
 * @param str - string 指针
 * @param new_size - string 新的大小
 * @return - 返回 0 表示成功， -1 表示失败， 1 表示新的空间小于原空间
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
	// 功能号
	int funcno;
	// 返回码
	int retcode;
	// 返回信息
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
