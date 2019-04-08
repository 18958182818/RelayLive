#pragma once

#include "ocilib.h"
#include "RowCollector.h"
#include "OracleInsert.h"

//��ȡ����
#define DBTOOL_CREATE_POOL(t,s) dbTool::Connect(t,s)
#define DBTOOL_GET_CONNECT(t)   dbTool::GetConnection(t)

//��ѯ
#define DBTOOL_GET_STRING(rs,i) dbTool::oci_get_string(rs, i).c_str()
#define DBTOOL_GET_INT(rs,i)    dbTool::oci_get_int(rs, i)
#define DBTOOL_GET_ODT(rs,i)    dbTool::oci_get_date(rs, i)
#define DBTOOL_GET_BLOB(rs,i)   dbTool::oci_get_blob(rs,i)

//����
#define DBTOOL_INSERTER         OracleInsert

namespace dbTool
{
    struct ConPool_Setting
    {
        std::string database;
        std::string username;
        std::string password;
        int max_conns;
        int min_conns;
        int inc_conns;  //����ʱһ��������������
    };

    bool Init(const char *path = NULL, void *lua = NULL);

    void Cleanup();

    bool Connect(std::string tag, ConPool_Setting settings);

    OCI_Connection* GetConnection(std::string tag);

    //��ѯ��¼�е��ַ���
    string oci_get_string(OCI_Resultset* rs, unsigned int i);

    //��ѯ��¼�е�����
    int oci_get_int(OCI_Resultset* rs, unsigned int i);

    //��ѯ��¼�е�ʱ��
    string oci_get_date(OCI_Resultset* rs, unsigned int i);

    //��ѯ��¼�ж�ȡ����������
    string oci_get_blob(OCI_Resultset* rs, unsigned int i);
}