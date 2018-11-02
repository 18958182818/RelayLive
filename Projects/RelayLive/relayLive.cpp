// sever.cpp : �������̨Ӧ�ó������ڵ㡣
//
#include "stdafx.h" 
#include "MiniDump.h"
#include "uv.h"
#include <thread>

int main()
{
    /** Dump���� */
    CMiniDump dump("gb28181_video_control.dmp");

    /** ������־�ļ� */
    char path[MAX_PATH];
    sprintf_s(path, MAX_PATH, ".\\log\\gb28181_video_control.txt");
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

    /** ��ʼ��SIP������ */
    if (!SipInstance::Init())
    {
        Log::error("SipInstance init failed");
        return -1;
    }
    Log::debug("SipInstance::Init ok");

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
    HttpWsServer::Init((void**)&p_loop_uv);

    /** ����һ��rtsp������ */
    CRtspServer rtsp;
    rtsp.Init(p_loop_uv);

    Log::debug("GB28181 Sever start success\r\n");

    // �¼�ѭ��
    while(true)
    {
        uv_run(p_loop_uv, UV_RUN_DEFAULT);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    Sleep(INFINITE);
    return 0;
}