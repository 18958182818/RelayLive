// sever.cpp : �������̨Ӧ�ó������ڵ㡣
//
#include "util.h"
#include "DeviceMgr.h"
#include "SipServer.h"
#include "MiniDump.h"
#include "ipc.h"
#include "utilc_api.h"
#include "stdio.h"



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
    {
        Log::error("�����ļ�����");
        return -1;
    }
    Log::debug("Settings::loadFromProfile ok");

    /** ���̼�ͨ�� */
    IPC::Init();

    /** ��ʼ���豸ģ�� */
    if (!DeviceMgr::Init())
    {
        Log::error("DeviceManagerInstance init failed");
        return -1;
    }
    Log::debug("DeviceMgr::Init ok");

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