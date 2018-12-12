// sever.cpp : �������̨Ӧ�ó������ڵ㡣
//
#include "common.h"
#include "DeviceMgr.h"
#include "MiniDump.h"
#include "uvIpc.h"
#include "util_api.h"
#include "stdio.h"

void on_ipc_recv(uv_ipc_handle_t* h, void* user, char* name, char* msg, char* data, int len)
{

}

int main()
{
    /** Dump���� */
    CMiniDump dump("rtsp_trans.dmp");

    /** ���̼�ͨ�� */
    uv_ipc_handle_t* h = NULL;
    int ret = uv_ipc_client(&h, "relay_live", NULL, "liveSrc", on_ipc_recv, NULL);
    if(ret < 0) {
        printf("ipc server err: %s\n", uv_ipc_strerr(ret));
    }

    /** ������־�ļ� */
    char path[MAX_PATH];
    sprintf_s(path, MAX_PATH, ".\\log\\rtspTrans.txt");
    Log::open(Log::Print::both, Log::Level::debug, path);

    /** ���������ļ� */
    if (!Settings::loadFromProfile(".\\config.txt"))
    {
        Log::error("�����ļ�����");
        return -1;
    }
    Log::debug("Settings::loadFromProfile ok");


    /** ��ʼ���豸ģ�� */
    if (!DeviceMgr::Init())
    {
        Log::error("DeviceManagerInstance init failed");
        return -1;
    }
    
    sleep(INFINITE);
    return 0;
}