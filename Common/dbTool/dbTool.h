#pragma once

#include "ocilib.h"
#include "RowCollector.h"
#include "OracleInsert.h"

//��ȡ����
#define OCI_CREATE_POOL(t,s) dbTool::Connect(t,s)
#define OCI_GET_CONNECT(t)   dbTool::GetConnection(t)

//��ѯ
#define OCI_GET_STRING(rs,i) dbTool::oci_get_string(rs, i).c_str()
#define OCI_GET_INT(rs,i)    OCI_IsNull(rs, i)?0:OCI_GetInt(rs, i)
#define OCI_GET_ODT(rs,i)    dbTool::oci_get_date(rs, i)
#define OCI_GET_BLOB(rs,i)   dbTool::oci_get_blob(rs,i)

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

    bool Init(const char *path = NULL);

    void Cleanup();

    bool Connect(std::string tag, ConPool_Setting settings);

    OCI_Connection* GetConnection(std::string tag);

    //��ѯ��¼�е��ַ���
    string oci_get_string(OCI_Resultset* rs,unsigned int i);

    //��ѯ��¼�е�ʱ��
    string oci_get_date(OCI_Resultset* rs,unsigned int i);

    //��ѯ��¼�ж�ȡ����������
    string oci_get_blob(OCI_Resultset* rs,unsigned int i);
}