// sever.cpp : �������̨Ӧ�ó������ڵ㡣
//
#include "common.h"
#include "HttpServer.h"
#include "RtspServer.h"
#include <windows.h>
#include "MiniDump.h"
//#include "uvIpc.h"
#include "uv.h"
//#include <thread>

int main()
{
    /** Dump���� */
    CMiniDump dump("relayLive.dmp");

    /** ������־�ļ� */
    char path[MAX_PATH];
    sprintf_s(path, MAX_PATH, ".\\log\\relayLive.txt");
    Log::open(Log::Print::both, Log::Level::debug, path);

    /** ���������ļ� */
    if (!Settings::loadFromProfile(".\\config.txt"))
    {
        Log::error("�����ļ�����");
        return -1;
    }
    Log::debug("Settings::loadFromProfile ok");

    //����cpu��������libuv�̳߳ص��߳�����
    static uv_loop_t *p_loop_uv = nullptr;
    uv_cpu_info_t* cpu_infos;
    int count;
    int err = uv_cpu_info(&cpu_infos, &count);
    if (err) {
        Log::warning("fail get cpu info: %s",uv_strerror(err));
    } else {
        wchar_t szCpuNum[10] = {0};
        swprintf_s(szCpuNum,L"%d", count);
        //���û���������ֵ
        ::SetEnvironmentVariableW(L"UV_THREADPOOL_SIZE",szCpuNum); 
    }

    //ȫ��loop
    p_loop_uv = uv_default_loop();

    /** ����һ��http������ */
    HttpWsServer::Init((void*)p_loop_uv);

    /** ����һ��rtsp������ */
    CRtspServer rtsp;
    rtsp.Init(p_loop_uv);

    Log::debug("GB28181 Sever start success\r\n");

    // �¼�ѭ��
    while(true)
    {
        uv_run(p_loop_uv, UV_RUN_DEFAULT);
        Sleep(1000);
    }
    Sleep(INFINITE);
    return 0;
}