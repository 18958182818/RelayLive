#pragma once
#include "Singleton.h"
#include "ConnPool.h"
#include "OciDefine.h"

//��ȡ����
#define GET_CONNECT_POOL(s)  OracleClient::GetInstance()->get_conn_pool(s)

//��ѯ
#define OCI_GET_STRING(rs,i) OCI_IsNull(rs, i)?"":OCI_GetString(rs, i)
#define OCI_GET_INT(rs,i)    OCI_IsNull(rs, i)?0:OCI_GetInt(rs, i)
#define OCI_GET_ODT(rs,i)    OracleClient::oci_get_date(rs, i)
#define OCI_GET_BLOB(rs,i)   OracleClient::oci_get_blob(rs,i)

struct ConPool_Setting
{
    std::string database;
    std::string username;
    std::string password;
    int max_conns;
    int min_conns;
    int inc_conns;  //����ʱһ��������������
};

class LIBOCI_API OracleClient : public Singleton<OracleClient>
{
    friend class Singleton<OracleClient>;
    OracleClient(void);
public:
    ~OracleClient(void);

    /**
     * ��������ȥ�������ݿ�
     * @param tag ���ݿ��ǩ
     * @param settings���ݿ�����
     */
    bool connect(std::string tag, ConPool_Setting settings);

    conn_pool_ptr get_conn_pool(string tag);

    //��ѯ��¼�е�ʱ��
    static string oci_get_date(OCI_Resultset* rs,unsigned int i);

    //��ѯ��¼�ж�ȡ����������
    static string oci_get_blob(OCI_Resultset* rs,unsigned int i);

private:
    std::map<std::string, conn_pool_ptr> conn_pools_;
};

