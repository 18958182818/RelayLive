#include "stdafx.h"
#include "DataBase.h"
#include "dbTool.h"
#include "luapp.hpp"


CDataBase::CDataBase(void)
    : m_strDB("DB")
{
}

CDataBase::~CDataBase(void)
{
    lua::GlobalFunction<lua::Bool()> luafCleanup;
    m_lua.getFunc("Cleanup",&luafCleanup);
    luafCleanup();
}

void CDataBase::Init()
{
    string path = Settings::getValue("DataBase", "Path");
    if(!dbTool::Init(path.c_str(), (void*)&m_lua))
        return;

    m_lua.run(".","DeviceMgr.lua");
    lua::GlobalFunction<lua::Bool()> luafInit;
    m_lua.getFunc("Init",        &luafInit);
    m_lua.getFunc("GetDevInfo",  &luafGetDevInfo);
    m_lua.getFunc("UpdateStatus",&luafUpdateStatus);
    m_lua.getFunc("UpdatePos",   &luafUpdatePos);
    m_lua.getFunc("InsertDev",   &luafInsertDev);
    luafInit();
}

vector<DevInfo*> CDataBase::GetDevInfo()
{
    vector<DevInfo*> vecRet;
    lua::Table rows = luafGetDevInfo();
    for(auto it = rows.getBegin(); !it.isEnd(); it++){
        lua::Var k,v;
        it.getKeyValue(&k, &v);
        if(!lua::VarType<lua::Table>(v))
            continue;
        DevInfo *dev = new DevInfo;
        lua::Table row = lua::VarCast<lua::Table>(v);
        for(auto rit = row.getBegin(); !rit.isEnd(); rit++){
            lua::Var k2,v2;
			rit.getKeyValue(&k2, &v2);
            lua::Str key = lua::VarCast<lua::Str>(k2);
            if(key == "DevID"){
				lua::Str value = lua::VarCast<lua::Str>(v2);
                dev->strDevID = value;
			} else if(key == "Name"){
				lua::Str value = lua::VarCast<lua::Str>(v2);
                dev->strName = value;
			} else if(key == "Status"){
				lua::Int value = lua::VarCast<lua::Int>(v2);
                dev->strStatus = value?"OFF":"ON";
            }
        }
        vecRet.push_back(dev);
    }
    return vecRet;
}

bool CDataBase::UpdateStatus(string code, bool online)
{
	int ret = online?1:0;
    return luafUpdateStatus((void*)code.c_str(), (void*)&ret);
}

bool CDataBase::UpdatePos(string code, string lat, string lon)
{
    if(lat.size() > 9) lat = lat.substr(0, 9);
    if(lon.size() > 9) lon = lon.substr(0, 9);
    return luafUpdatePos((void*)code.c_str(), (void*)&lat, (void*)&lon);
}

bool CDataBase::InsertDev(DevInfo* dev)
{
    lua::Table tb;
    tb[1] = dev->strDevID;
    tb[2] = dev->strStatus=="ON"?"1":"0";
    tb[3] = dev->strLatitude;
    tb[4] = dev->strLongitude;
    return luafInsertDev(tb);
}