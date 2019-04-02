#pragma once
#include "OracleInsert.h"

/**
 * ���ݿ��һ�еĵ�Ԫ����
 */
struct oci_column
{
    string      strColumnName;  //< ������
    uint32_t    nColumnType;    //< ������
    uint32_t    nMaxLength;     //< ��󳤶ȣ��������ַ���ʱ��������
    bool        bNullable;      //< �Ƿ��Ϊ��
    string      strDefault;     //< Ĭ��ֵ��ż����Ҫ
};

/**
 * ��װOracle�������ݵĲ���
 */
class OracleInsert
{
public:
    OracleInsert();
    OracleInsert(string strDB, string strTable, int nRowSize = 100);
    ~OracleInsert(void);

    /**
     * ��ʼ�����ݿ�����
     * @param strDB ������������
     * @param strTable ������
     * @param nRowSize һ���ύ����������
     */
    void Init(string strDB, string strTable, int nRowSize);

    /**
     * ���ݿ�����һ���ֶ�
     * @param strColumnName ������
     * @param nColumnType ����
     * @param nMaxLength ����Ϊ�ַ���ʱ����Ҫ���ó���
     * @param bNullable ������Ϊ��ʱ������������������Ϊ�գ����ֶ�����Ϊ��
     * @param strDefault Ĭ��ֵ����������Ϊ�գ��Ҳ������������Ϊ�գ����ֶ���ΪĬ��ֵ
     */
    void AddCloumn(string strColumnName, 
        uint32_t nColumnType, 
        uint32_t nMaxLength = 16, 
        bool bNullable = false, 
        string strDefault = "");

    /**
     * Ԥ����������AddCloumn������ʹ��
     * ����sql�����������ݵĿռ�
     */
    bool Prepair();

    /**
     * ������������
     * @param rows �������ݣ�ÿ�е��ֶθ���Ӧ�������õ���ͬ
     */
    bool Insert(vector<vector<string>> rows);

private:
    //��Ҫ��֤target�ռ��㹻, lenΪ������'\0'�ĳ���ֵ
    static void str_set(char *target, int index, int len, const char *source);
    static void bind_set_data(OCI_Bind* bind, int index, int len, string val, bool is_null = false);
    static void bind_set_data(OCI_Bind* bind, int index, string val, string date_fmt, bool is_null = false);
    static void bind_set_data(OCI_Bind* bind, int index, int16_t val, bool is_null = false);
    static void bind_set_data(OCI_Bind* bind, int index, int32_t val, bool is_null = false);
    static void bind_set_data(OCI_Bind* bind, int index, int64_t val, bool is_null = false);

private:
    string                  m_strDataBase;      //< ���ݿ���������
    string                  m_strTableName;     //< Ҫ����ı�����
    vector<oci_column>      m_vecTableRow;      //< ����

    char*                   m_buff;             //< �ڴ�����
    int                     m_nPointLen;        //< ָ��ĳ��ȣ�32λƽ̨4�ֽڣ�64λƽ̨8�ֽ�
    int                     m_nRowSize;         //< һ���ύ��������
    string                  m_strSql;           //< ִ�в����sql���
};

