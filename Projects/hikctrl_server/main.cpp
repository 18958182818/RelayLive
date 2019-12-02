// sever.cpp : �������̨Ӧ�ó������ڵ㡣
//
#include "util.h"
#include "server.h"
#include <windows.h>
#include "MiniDump.h"
#include "uv.h"

int main()
{
    /** Dump���� */
    CMiniDump dump("hikctrl_server.dmp");

    /** ������־�ļ� */
    char path[MAX_PATH];
    sprintf_s(path, MAX_PATH, ".\\log\\hikctrl_server.txt");
    Log::open(Log::Print::both, Log::Level::debug, path);
    Log::debug("version: %s %s", __DATE__, __TIME__);

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
        char szThreadNum[10] = {0};
        sprintf(szThreadNum, "%d", count*2+1);
        Log::debug("thread pool size is %s", szThreadNum);
        //���û���������ֵ
        //::SetEnvironmentVariableW(L"UV_THREADPOOL_SIZE",szCpuNum); 
        uv_os_setenv("UV_THREADPOOL_SIZE", szThreadNum);
    }
    uv_free_cpu_info(cpu_infos, count);

    //ȫ��loop
    p_loop_uv = uv_default_loop();

    /** ����һ��http������ */
    Server::Init((void*)p_loop_uv);

    Log::debug("web Sever start success\r\n");

    // �¼�ѭ��
    while(true)
    {
        uv_run(p_loop_uv, UV_RUN_DEFAULT);
        Sleep(1000);
    }
    Sleep(INFINITE);
    return 0;
}