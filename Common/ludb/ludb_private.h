#ifndef _LUDB_PRIVATE_H_
#define _LUDB_PRIVATE_H_

#include "ludb_public.h"
#include "cstl.h"
#include "cstl_easy.h"
#include "uv.h"

#ifdef __cplusplus
extern "C" {
#endif

/** ���Ӿ�� */
typedef struct _ludb_conn_ {
    ludb_db_type_t type;
    bool           from_pool;
    void          *conn;
}ludb_conn_t;

/** statement��� */
typedef struct _ludb_stmt_ {
    ludb_db_type_t type;
    ludb_conn_t   *conn;
    void          *stmt;
}ludb_stmt_t;

/** resultset��� */
typedef struct _ludb_rest_ {
    ludb_db_type_t type;
    ludb_conn_t   *conn;
    ludb_stmt_t   *stmt;
    void          *rest;
}ludb_rest_t;

/** ���ӳؾ�� */
typedef struct _ludb_pool_ {
    ludb_db_type_t type;
    void          *pool;
}ludb_pool_t;

/** �ֶζ��� */
typedef struct _bind_column_pri_ {
    string_t        *name;        //< �󶨱�����ƻ�������
    column_type_t    type;        //< ������
    int              max_len;     //< ��󳤶ȣ��������ַ���ʱ��������
    bool             nullable;    //< �Ƿ��Ϊ��
    string_t        *default_value;     //< Ĭ��ֵ��ż����Ҫ
}bind_column_pri_t;

/** ���������� */
typedef struct _ludb_batch_ {
    ludb_db_type_t type;      //< ���ݿ�����
    string_t      *tag;       //< ���ӳر�ǩ
    string_t      *sql;       //< ִ�е�sql���
    int            row_num;   //< ���ݻ���ﵽ��ô���о�ִ��
    int            interval;  //< ���ϴ�ִ�о�����ô���룬�������ݲ�Ϊ�վ�ִ�С�0 ��������ʱ��
    vector_t      *binds;     //< vector<bind_column_t>
    queue_t       *recvs;     //< queue<vector<string>> ���ݽ��ն���
    vector_t      *insts;     //< vector<vector<string>> ����������
    uv_mutex_t     mutex;     //< ���������������л�����
    bool           running;   //< �Ƿ�����ִ��
    time_t         ins_time;  //< ��һ���ύ��ʱ��
    uv_thread_t    tid;       //< ִ���߳�ID
    void          *handle;    //< ���ݲ�ͨ���ͷֱ�����ԵĽṹ
}ludb_batch_t;

extern LOG_HANDLE g_log_hook;
extern int pointLen;

#ifdef __cplusplus
}
#endif
#endif