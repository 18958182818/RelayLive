// sever.cpp : �������̨Ӧ�ó������ڵ㡣
//
#include "common.h"
#include "MiniDump.h"
#include "utilc_api.h"
#include "HttpServer.h"
#include "DeviceInfo.h"
#include "uv.h"
#include <stdio.h>

int main()
{
    /** Dump���� */
    CMiniDump dump("rtsp2ws.dmp");

    /** ������־�ļ� */
    char path[MAX_PATH];
    sprintf_s(path, MAX_PATH, ".\\log\\rtsp2ws.txt");
    Log::open(Log::Print::both, Log::Level::debug, path);

    /** ���������ļ� */
    if (!Settings::loadFromProfile(".\\config.txt"))
    {
        Log::error("�����ļ�����");
        return -1;
    }
    Log::debug("Settings::loadFromProfile ok");

    //ȫ��loop
    static uv_loop_t *p_loop_uv = nullptr;
    p_loop_uv = uv_default_loop();

    /** ����һ��http������ */
    HttpWsServer::Init((void*)p_loop_uv);
    Log::debug("GB28181 Sever start success\r\n");

    /** ���ݿ�ģ�� */
    DeviceInfo::Init();
    Log::debug("Get devices from oracle success\r\n");

    // �¼�ѭ��
    while(true)
    {
        uv_run(p_loop_uv, UV_RUN_DEFAULT);
        Sleep(1000);
    }
    sleep(INFINITE);
    return 0;
}