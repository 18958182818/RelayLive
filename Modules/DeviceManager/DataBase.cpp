#include "stdafx.h"
#include "DataBase.h"
#include "libOci.h"


CDataBase::CDataBase(void)
    : m_strDB("DB")
{
}

CDataBase::~CDataBase(void)
{
}

void CDataBase::init()
{
    auto oci_error_handler = [](OCI_Error *err){
        OCI_Connection * conn = OCI_ErrorGetConnection(err);
        Log::error("Error[ORA-%05d] - msg[%s] - database[%s] - user[%s] - sql[%s]"
            , OCI_ErrorGetOCICode(err)
            , OCI_ErrorGetString(err)
            , OCI_GetDatabase(conn)
            , OCI_GetUserName(conn)
            , OCI_GetSql(OCI_ErrorGetStatement(err)));
    };
    string path = Settings::getValue("DataBase", "Path");

    if(!OCI_Initialize(oci_error_handler, path.c_str(), OCI_ENV_THREADED))
        return;

    ConPool_Setting dbset;
    dbset.database = Settings::getValue("DataBase","Addr");
    dbset.username = Settings::getValue("DataBase","User");
    dbset.password = Settings::getValue("DataBase","PassWord");
    dbset.max_conns = 5;
    dbset.min_conns = 2;
    dbset.inc_conns = 2;
    OracleClient* client = OracleClient::GetInstance();
    client->connect(m_strDB, dbset);

    string tableName = Settings::getValue("DataBase","TableName");
    string colCode = Settings::getValue("DataBase","ColumnCODE");
    string colStat = Settings::getValue("DataBase","ColumnSTATUS");
    string colLon = Settings::getValue("DataBase","ColumnLON");
    string colLat = Settings::getValue("DataBase","ColumnLAT");

    stringstream ss;
    ss << "select " << colCode << "," << colStat << " from  " << tableName;
    m_strGetDevsSql = ss.str();
    ss.clear();
    ss.str();
    ss << "update " << tableName << " set " << colStat << " = :status where " << colCode << " = :code";
    m_strUpdateStatSql = ss.str();
    ss.clear();
    ss.str();
    ss << "update " << tableName << " set " << colLat << " = :lat, " 
        << colLon << " = :lon where " << colCode << " = :code";
    m_strUpdatePosSql == ss.str();
}

vector<DevInfo*> CDataBase::GetDevInfo()
{
    vector<DevInfo*> vecRet;
    conn_pool_ptr pool = OracleClient::GetInstance()->get_conn_pool(m_strDB);
    if (pool == NULL) {
        Log::error("fail to get pool: %s", m_strDB.c_str());
        return vecRet;
    }
    int index = pool->getConnection();
    if (index == -1) {
        Log::error("fail to get connection: %s", m_strDB.c_str());
        return vecRet;
    }
    OCI_Connection *cn = pool->at(index);
    OCI_Statement *st = OCI_CreateStatement(cn);
    OCI_ExecuteStmt(st, m_strGetDevsSql.c_str());
    OCI_Resultset *rs = OCI_GetResultset(st);
    while (OCI_FetchNext(rs)) 
    {
        DevInfo* dev = new DevInfo;
        dev->strDevID     = OCI_GET_STRING(rs,1);
        dev->strStatus    = OCI_GET_INT(rs,2)>0?"ON":"OFF";
        vecRet.push_back(dev);
    }
    return vecRet;
}

bool CDataBase::UpdateStatus(string code, bool online)
{
    conn_pool_ptr pool = OracleClient::GetInstance()->get_conn_pool(m_strDB);
    if (pool == NULL) {
        Log::error("fail to get pool: %s", m_strDB.c_str());
        return false;
    }
    int index = pool->getConnection();
    if (index == -1) {
        Log::error("fail to get connection: %s", m_strDB.c_str());
        return false;
    }
    int nStateValue = online?1:0;
    OCI_Connection *cn = pool->at(index);
    OCI_Statement *st = OCI_CreateStatement(cn);
    OCI_Prepare(st, m_strUpdateStatSql.c_str());
    OCI_BindInt(st, ":status",   &nStateValue);
    OCI_BindString(st, ":code", (char*)code.c_str(), 30);
    OCI_Execute(st);
    int count = OCI_GetAffectedRows(st);
    OCI_Commit(cn);
    OCI_FreeStatement(st);
    pool->releaseConnection(index);
    return true;
}

bool CDataBase::UpdatePos(string code, string lat, string lon)
{
    conn_pool_ptr pool = OracleClient::GetInstance()->get_conn_pool(m_strDB);
    if (pool == NULL) {
        Log::error("fail to get pool: %s", m_strDB.c_str());
        return false;
    }
    int index = pool->getConnection();
    if (index == -1) {
        Log::error("fail to get connection: %s", m_strDB.c_str());
        return false;
    }
    if(lat.size() > 9) lat = lat.substr(0, 9);
    if(lon.size() > 9) lon = lon.substr(0, 9);
    OCI_Connection *cn = pool->at(index);
    OCI_Statement *st = OCI_CreateStatement(cn);
    OCI_Prepare(st, m_strUpdatePosSql.c_str());
    OCI_BindString(st, ":lat", (char*)lat.c_str(), 10);
    OCI_BindString(st, ":lon", (char*)lon.c_str(), 10);
    OCI_BindString(st, ":code", (char*)code.c_str(), 30);
    OCI_Execute(st);
    int count = OCI_GetAffectedRows(st);
    OCI_Commit(cn);
    OCI_FreeStatement(st);
    pool->releaseConnection(index);
    return true;
}