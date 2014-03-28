/******************************************************************************
* Copyright SupWisdom 2012 all right reserved
* Module: 一卡通后台通讯协议封装
* File: message.c
* Auth: Cheng Tang<cheng.tang@supwisdom.com>
******************************************************************************/
#include <assert.h>
#include "message.h"
#include "p16log.h"
#include "config.h"

#define DEFAULT_STRING_LEN 256

static int do_new_string(sw_string_t* str, int newsize)
{
    char* s = (char*)malloc(newsize + 1);
    if (NULL == s)
        return -1;
    str->str = s;
    str->max_len = newsize;
    str->use_len = 0;
    return 0;
}

int sw_new_string(sw_string_t* str)
{
    return do_new_string(str, DEFAULT_STRING_LEN);
}

void sw_free_string(sw_string_t* str)
{
    if (str->str)
    {
        free(str->str);
        str->str = NULL;
    }
    str->max_len = 0;
    str->use_len = 0;
}

int sw_string_extend(sw_string_t* str, uint32 ext_size)
{
    uint32 new_size = str->max_len + ext_size;
    if (str->str)
    {
        str->str = (char*)realloc(str->str, new_size + 1);
        if (NULL == str->str)
            return -1;
        str->max_len = new_size;
        return 0;
    }
    return do_new_string(str, new_size);
}

int sw_string_new_size(sw_string_t* str, uint32 new_size)
{
    if (str->max_len > new_size)
        return 1;
    if (str->max_len == new_size)
        return 0;
    uint32 e = new_size - str->max_len;
    return sw_string_extend(str, e);
}

int sw_string_cmp(const sw_string_t* str1, const sw_string_t* str2)
{
    assert(str1->str != NULL && str2->str != NULL);
    return strcmp(str1->str, str2->str);
}
int sw_string_cmp_n(const sw_string_t* str1, const sw_string_t* str2, uint32 n)
{
    assert(str1->str != NULL && str2->str != NULL);
    return strncmp(str1->str, str2->str, n);
}
int sw_string_copy(sw_string_t* dest_str, const sw_string_t* src_str)
{
    return sw_string_copy_n(dest_str, src_str, src_str->use_len);
}
int sw_string_copy_n(sw_string_t* dest_str, const sw_string_t* src_str, uint32 n)
{
    assert(n <= src_str->use_len);
    if (n > dest_str->max_len)
    {
        if (sw_string_new_size(dest_str, n))
            return -1;
    }
    memcpy(dest_str->str, src_str->str, n);
    dest_str->use_len = n;
    return 0;
}
int sw_string_copy_c(sw_string_t* dest_str, const char* src)
{
    return sw_string_copy_cn(dest_str, src, strlen(src));
}
int sw_string_copy_cn(sw_string_t* dest_str, const char* src, uint32 n)
{
    if (n > dest_str->max_len)
    {
        if (sw_string_new_size(dest_str, n))
            return -1;
    }
    memcpy(dest_str->str, src, n);
    dest_str->use_len = n;
    return 0;
}
int sw_string_cat(sw_string_t* dest_str, const sw_string_t* src_str)
{
    return sw_string_cat_n(dest_str, src_str, src_str->use_len);
}
int sw_string_cat_n(sw_string_t* dest_str, const sw_string_t* src_str, uint32 n)
{
    assert(n <= src_str->use_len);
    uint new_size = n + dest_str->use_len;
    if (n > dest_str->max_len)
    {
        if (sw_string_new_size(dest_str, new_size))
            return -1;
    }
    memcpy(dest_str->str + dest_str->use_len, src_str->str, n);
    dest_str->use_len = new_size;
    return 0;
}
int sw_string_cat_c(sw_string_t* dest_str, const char* src)
{
    return sw_string_cat_cn(dest_str, src, strlen(src));
}
int sw_string_cat_cn(sw_string_t* dest_str, const char* src, uint32 n)
{
    uint new_size = n + dest_str->use_len;
    if (n > dest_str->max_len)
    {
        if (sw_string_new_size(dest_str, new_size))
            return -1;
    }
    memcpy(dest_str->str + dest_str->use_len, src, n);
    dest_str->use_len = new_size;
    return 0;
}
void sw_string_finalize(sw_string_t* str)
{
    if (str->str)
    {
        str->str[str->use_len] = 0;
    }
}
////////////////////////////////////////////////////////////////////////////////

#define EASY_DELETE_JSON(obj) do{ if(obj!=NULL){ cJSON_Delete(obj); } } while(0)
#if 0
static cJSON* find_child_by_name(const cJSON* root, const char* name)
{
    return cJSON_GetObjectItem((cJSON*)root, name);
}
#endif
int sw_message_reset(sw_message_t* msg)
{
    msg->funcno = 0;
    msg->retcode = 0;
    if (msg->retmsg.str)
    {
        msg->retmsg.use_len = 0;
    }
    else
    {
        sw_new_string(&(msg->retmsg));
    }
    if (msg->root)
    {
        cJSON_Delete(msg->root);
        msg->root = NULL;
    }
    if (msg->current_row)
    {
        cJSON_Delete(msg->current_row);
        msg->current_row = NULL;
    }
    msg->root = cJSON_CreateObject();
    msg->frozen = 0;
    msg->fetch_rownum = -1;
    return 0;
}
void sw_message_free(sw_message_t* msg)
{
    sw_free_string(&(msg->retmsg));
    EASY_DELETE_JSON(msg->current_row);
    EASY_DELETE_JSON(msg->root);
}

int sw_message_col_define(sw_message_t* msg, const char* colnames[], int colcount)
{
    assert(msg->frozen == 0);
    assert(msg->root != NULL);
    int i;
    cJSON_DeleteItemFromObject(msg->root, "colname");
    cJSON_DeleteItemFromObject(msg->root, "colcnt");
    cJSON* colname_obj = cJSON_CreateArray();
    for (i = 0; i < colcount; ++i)
    {
        cJSON_AddItemToArray(colname_obj, cJSON_CreateString(colnames[i]));
    }
    cJSON_AddItemReferenceToObject(msg->root, "colname", colname_obj);
    cJSON_AddNumberToObject(msg->root, "colcnt", cJSON_GetArraySize(colname_obj));
    cJSON_AddItemReferenceToObject(msg->root, "coldesc", colname_obj);
    //cJSON_Delete(colname_obj);
    //printf(cJSON_Print(msg->root));
    return 0;
}
int sw_message_serialize(sw_message_t* msg, sw_string_t* str)
{
    msg->frozen = 1;
    cJSON_AddNumberToObject(msg->root, "funcno", msg->funcno);
    cJSON* rowdata = cJSON_GetObjectItem(msg->root, "rowdata");
    int size = 0;
    if(rowdata != NULL)
    {
        size = cJSON_GetArraySize(rowdata);
    }
    cJSON* rowcnt = cJSON_CreateNumber(size);
    cJSON_AddItemToObject(msg->root, "rowcnt", rowcnt);
    char* value = cJSON_PrintUnformatted(msg->root);
    if (NULL == value)
        return -1;
    sw_string_copy_c(str, value);
    sw_string_finalize(str);
    free(value);
    return 0;
}
int sw_message_unserialize(const sw_string_t* str, sw_message_t* msg)
{
    assert(str->str != NULL);
    msg->frozen = 1;
    str->str[str->use_len] = 0;
    sw_message_reset(msg);
    msg->root = cJSON_Parse(str->str);
    if (NULL == msg->root)
    {
        LOG((LOG_ERROR, "不能解析JSON报文"));
        return -1;
    }

    cJSON* elem = cJSON_GetObjectItem(msg->root, "retcode");
    if (elem == NULL || elem->type != cJSON_Number)
    {
        LOG((LOG_ERROR, "JSON报文不包含retcode属性或类型错误"));
        return -1;
    }
    msg->retcode = elem->valueint;
    if (msg->retcode != 0)
    {
        elem = cJSON_GetObjectItem(msg->root, "retmsg");
        if (elem == NULL || elem->type != cJSON_String)
        {
            LOG((LOG_ERROR, "JSON报文不包含retmsg属性或类型错误"));
            return -1;
        }
        sw_string_copy_c(&(msg->retmsg), elem->valuestring);
        sw_string_finalize(&msg->retmsg);
    }
    elem = cJSON_GetObjectItem(msg->root, "funcno");
    if (elem == NULL || elem->type != cJSON_Number)
    {
        LOG((LOG_ERROR, "JSON报文不包含funcno属性或类型错误"));
        return -1;
    }
    msg->funcno = elem->valueint;

    if (msg->current_row != NULL)
        cJSON_Delete(msg->current_row);

    msg->current_row = NULL;

    msg->fetch_rownum = -1;
    msg->frozen = 1;
    return 0;
}
int sw_message_set_attr(sw_message_t* msg, const char* attr_name, const char* attr_value)
{
    assert(msg->frozen == 0);
    cJSON* attr = cJSON_GetObjectItem(msg->root, attr_name);
    if (NULL != attr)
    {
        cJSON_Delete(attr);
    }
    cJSON_AddStringToObject(msg->root, attr_name, attr_value);
    return 0;
}
int sw_message_set_attr_int(sw_message_t* msg, const char* attr_name, int attr_value)
{
    assert(msg->frozen == 0);
    cJSON* attr = cJSON_GetObjectItem(msg->root, attr_name);
    if (NULL != attr)
    {
        cJSON_Delete(attr);
    }
    cJSON_AddNumberToObject(msg->root, attr_name, attr_value);
    return 0;
}
int sw_message_set_row(sw_message_t* msg, const char* col_name, const char* col_value)
{
    assert(msg->frozen == 0);
    if (msg->current_row == NULL)
    {
        msg->current_row = cJSON_CreateObject();
    }
    if (NULL == msg->current_row)
    {
        LOG((LOG_ERROR, "不能增加行"));
        return -1;
    }
    cJSON* elem = cJSON_GetObjectItem(msg->current_row, col_name);
    if (NULL != elem)
    {
        cJSON_Delete(elem);
    }
    cJSON_AddStringToObject(msg->current_row, col_name, col_value);
    return 0;
}
int sw_message_set_row_int(sw_message_t* msg, const char* col_name, int col_value)
{
    assert(msg->frozen == 0);
    if (msg->current_row == NULL)
    {
        msg->current_row = cJSON_CreateObject();
    }
    if (NULL == msg->current_row)
    {
        LOG((LOG_ERROR, "不能增加行"));
        return -1;
    }
    cJSON* elem = cJSON_GetObjectItem(msg->current_row, col_name);
    if (NULL != elem)
    {
        cJSON_Delete(elem);
    }
    cJSON_AddNumberToObject(msg->current_row, col_name, col_value);
    return 0;
}
int sw_message_add_row(sw_message_t* msg)
{
    int i;
    assert(msg->frozen == 0);
    assert(msg->current_row != NULL);
    cJSON* colname;
    cJSON* colvalues;
    colname = cJSON_GetObjectItem(msg->root, "colname");
    if (NULL == colname)
    {
        LOG((LOG_ERROR, "未设置colname属性"));
        return -1;
    }
    colvalues = cJSON_CreateArray();
    int colcnt = cJSON_GetArraySize(colname);
    for (i = 0; i < colcnt; ++i)
    {
        cJSON* col = cJSON_GetArrayItem(colname, i);
        assert(col->type == cJSON_String);
        cJSON* colval = cJSON_GetObjectItem(msg->current_row, col->valuestring);
        if (colval)
        {
            cJSON_AddItemReferenceToArray(colvalues, colval);
        }
        else
        {
            cJSON_AddItemToArray(colvalues, cJSON_CreateNull());
        }
    }
    cJSON* rowdata = cJSON_GetObjectItem(msg->root, "rowdata");
    /* cJSON* rowcnt = cJSON_GetObjectItem(msg->root, "rowcnt"); */
    if (NULL == rowdata)
    {
        rowdata = cJSON_CreateArray();
        /* rowcnt = cJSON_CreateNumber(1); */
        cJSON_AddItemToObject(msg->root, "rowdata", rowdata);
        /* cJSON_AddItemToObject(msg->root, "rowcnt", rowcnt); */
    }
    else
    {
        /* rowcnt->valueint++; */
    }
    cJSON_AddItemReferenceToArray(rowdata, colvalues);
    //cJSON_Delete(colvalues);
    //cJSON_Delete( msg->current_row );
    msg->current_row = NULL;
    return 0;
}
static int _get_cJSON_string_value(cJSON* obj, char* value, int maxlen)
{
    assert(obj != NULL);
    //printf("value type=%d\n",obj->type);
    switch (obj->type % cJSON_IsReference)
    {
    case cJSON_False:
        strncpy(value, "false", maxlen);
        break;
    case cJSON_True:
        strncpy(value, "true", maxlen);
        break;
    case cJSON_NULL:
        strncpy(value, "NULL", maxlen);
        break;
    case cJSON_Number:
        //LOG((LOG_DEBUG,"value=%d",obj->valueint));
        sprintf(value, "%d", obj->valueint);
        break;
    case cJSON_String:
        memcpy(value, obj->valuestring, maxlen);
        break;
    case cJSON_Array:
        return -1;
    case cJSON_Object:
        return -1;
    }
    return 0;
}
int sw_message_get_attr(sw_message_t* msg, const char* attr_name, char* attr_value,
                        int maxlen)
{
    assert(msg->frozen == 1);
    cJSON* elem = cJSON_GetObjectItem(msg->root, attr_name);
    if (NULL == elem)
        return -1;

    return _get_cJSON_string_value(elem, attr_value, maxlen);
}
int sw_message_get_attr_int(sw_message_t* msg, const char* attr_name, int* attr_value)
{
    char temp[64];
    int ret;
    memset(temp, 0, sizeof temp);
    ret = sw_message_get_attr(msg, attr_name, temp, sizeof(temp) - 1);
    if (ret)
    {
        return -1;
    }
    *attr_value = atoi(temp);
    return 0;
}
int sw_message_fetch_row(sw_message_t* msg)
{
    assert(msg->frozen == 1);
    cJSON* rowdata = cJSON_GetObjectItem(msg->root, "rowdata");
    if (NULL == rowdata)
        return -1;
    if (msg->current_row != NULL)
    {
        cJSON_Delete(msg->current_row);
        msg->current_row = NULL;
    }
    if (msg->fetch_rownum < cJSON_GetArraySize(rowdata))
    {
        msg->fetch_rownum++;

        cJSON* row = cJSON_GetArrayItem(rowdata, msg->fetch_rownum);
        if (NULL == row)
            return -1;
        cJSON* colname = cJSON_GetObjectItem(msg->root, "colname");
        assert(cJSON_GetArraySize(colname) == cJSON_GetArraySize(row));
        int i;
        msg->current_row =  cJSON_CreateObject();
        for (i = 0; i < cJSON_GetArraySize(colname); ++i)
        {
            cJSON* col = cJSON_GetArrayItem(colname, i);
            cJSON* val = cJSON_GetArrayItem(row, i);
            assert(col != NULL && val != NULL);
            assert(col->type == cJSON_String);
            cJSON_AddItemReferenceToObject(msg->current_row, col->valuestring, val);

        }
        //printf("ROW: %s",cJSON_Print(msg->current_row));
        return 0;
    }
    return -1;
}
int sw_message_get_row(sw_message_t* msg, const char* col_name, char* col_value,
                       int maxlen)
{
    assert(msg->frozen == 1);
    assert(msg->current_row != NULL);
    cJSON* elem = cJSON_GetObjectItem(msg->current_row, col_name);
    if (elem == NULL)
        return -1;
    return _get_cJSON_string_value(elem, col_value, maxlen);
}
int sw_message_get_row_int(sw_message_t* msg, const char* col_name, int* col_value)
{
    char temp[64];
    int ret;
    memset(temp, 0, sizeof temp);
    ret = sw_message_get_row(msg, col_name, temp, sizeof(temp) - 1);
    //LOG((LOG_DEBUG,"col=%s,val=%s",col_name,temp));
    if (ret)
    {
        return -1;
    }
    *col_value = atoi(temp);
    return 0;
}
