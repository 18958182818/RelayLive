// sever.cpp : �������̨Ӧ�ó������ڵ㡣
//
#include "util.h"
#include "utilc_api.h"
#include "MiniDump.h"
#include "ipc.h"
#include "SipServer.h"
#include "script.h"
#include <stdio.h>
#include <map>
#include <sstream>

std::map<string, SipServer::DevInfo*> g_mapDevice;
CriticalSection                        _csDevs;

static void on_device(SipServer::DevInfo* dev) {
	//Script::InsertDev(dev);
	MutexLock lock(&_csDevs);
	if(g_mapDevice.count(dev->strDevID) == 0)
	    g_mapDevice.insert(make_pair(dev->strDevID, dev));
	else {
	    delete g_mapDevice[dev->strDevID];
	    g_mapDevice[dev->strDevID] = dev;
	}
}

static void on_update_status(string strDevID, string strStatus) {
	//Script::UpdateStatus(strDevID, strStatus);
	MutexLock lock(&_csDevs);
	auto fit = g_mapDevice.find(strDevID);
	if(fit != g_mapDevice.end()) {
		fit->second->strStatus = strStatus;
	}
}

static void on_update_postion(string strDevID, string log, string lat) {
	//Script::UpdatePos(strDevID, lat, log);
	MutexLock lock(&_csDevs);
	auto fit = g_mapDevice.find(strDevID);
	if(fit != g_mapDevice.end()) {
		fit->second->strLongitude = log;
		fit->second->strLatitude = lat;
	}
}

int main()
{
    /** Dump���� */
    CMiniDump dump("sipServer.dmp");

    /** ������־�ļ� */
    char path[MAX_PATH];
    sprintf_s(path, MAX_PATH, ".\\log\\sipServer.txt");
    Log::open(Log::Print::both, Log::Level::debug, path);
    Log::debug("version: %s %s", __DATE__, __TIME__);

    /** ���������ļ� */
    if (!Settings::loadFromProfile(".\\config.txt"))
        Log::error("Settings::loadFromProfile failed");
    else
        Log::debug("Settings::loadFromProfile ok");

    /** ���̼�ͨ�� */
    IPC::Init();

	/** ���ݿ�ű� */
	//Script::Init();

	SipServer::SetDeviceCB(on_device);
	SipServer::SetUpdateStatusCB(on_update_status);
	SipServer::SetUpdatePostionCB(on_update_postion);

    /** ��ʼ��SIP������ */
    if (!SipServer::Init())
    {
        Log::error("SipInstance init failed");
        return -1;
    }
    Log::debug("SipInstance::Init ok");
    
    sleep(INFINITE);
    return 0;
}

std::string GetDevsJson() {
	MutexLock lock(&_csDevs);
	stringstream ss;
	ss << "{\"root\":[";
	bool first = true;
	for(auto c:g_mapDevice){  
		if(!first) {
			ss << ",";
		} else {
			first = false;
		}
		ss << FormatDevInfo(c.second, true);
	}
	ss << "]}";
	return ss.str();
}