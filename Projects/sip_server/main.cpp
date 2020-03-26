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

std::map<string, SipServer::DevInfo*> g_mapDevs;
CriticalSection                       g_csDevs;
bool                                  _useScript = false;   //�Ƿ�����lua�ű�

// ÿ��ִ��һ���������ݿ����
void on_clean_everyday(time_t t) {
    struct tm * timeinfo = localtime(&t);
    if(_useScript)
        Script::CleanDev(timeinfo->tm_hour);
}

// ��ѯĿ¼�õ��豸��ϢӦ��
void on_device(SipServer::DevInfo* dev) {
	if(_useScript)
        Script::InsertDev(dev);

	MutexLock lock(&g_csDevs);
	if(g_mapDevs.count(dev->strDevID) == 0)
	    g_mapDevs.insert(make_pair(dev->strDevID, dev));
	else {
	    delete g_mapDevs[dev->strDevID];
	    g_mapDevs[dev->strDevID] = dev;
	}
}

// �����豸����״̬
void on_update_status(string strDevID, string strStatus) {
    if(_useScript)
	    Script::UpdateStatus(strDevID, strStatus);

	MutexLock lock(&g_csDevs);
	auto fit = g_mapDevs.find(strDevID);
	if(fit != g_mapDevs.end()) {
		fit->second->strStatus = strStatus;
	}
}

// �����豸gps
void on_update_postion(string strDevID, string log, string lat) {
    if(_useScript)
	    Script::UpdatePos(strDevID, lat, log);

	MutexLock lock(&g_csDevs);
	auto fit = g_mapDevs.find(strDevID);
	if(fit != g_mapDevs.end()) {
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
    string use = Settings::getValue("Script", "use", "false");
    if(use == "yes" || use == "1")
        _useScript = true;
    if(_useScript)
	    Script::Init();

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
	MutexLock lock(&g_csDevs);
	stringstream ss;
	ss << "{\"root\":[";
	bool first = true;
	for(auto c:g_mapDevs){  
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