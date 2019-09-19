#ifndef _LUDB_PUBLIC_H_
#define _LUDB_PUBLIC_H_

#include "utilc.h"

#ifdef __cplusplus
extern "C" {
#endif

/** ���ݿ����� */
typedef enum _ludb_db_type {
    ludb_db_oracle = 0,
    ludb_db_mongo
} ludb_db_type_t;

/** connect��� */
typedef struct _ludb_conn_ ludb_conn_t;

/** statement��� */
typedef struct _ludb_stmt_ ludb_stmt_t;

/** resultset��� */
typedef struct _ludb_rest_ ludb_rest_t;

/** ��־�ص����� */
typedef void (*LOG_HANDLE)(char *log);

/** ���������� */
typedef struct _ludb_batch_ ludb_batch_t;

/** �ֶ����Ͷ��� */
typedef enum _column_type_ {
    column_type_char = 0,
    column_type_int,
    column_type_float,
    column_type_long,
    column_type_uint,
    column_type_blob,
    column_type_date
}column_type_t;

/** �ֶζ��� */
typedef struct _bind_column_ {
    char*            name;        //< �󶨱�����ƻ�������
    column_type_t    type;        //< ������
    int              max_len;     //< ��󳤶ȣ��������ַ���ʱ��������
    bool             nullable;    //< �Ƿ��Ϊ��
    char*            default_value;     //< Ĭ��ֵ��ż����Ҫ
}bind_column_t;

#ifdef __cplusplus
}
#endif
#endif