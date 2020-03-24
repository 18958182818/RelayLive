#pragma once

namespace Server
{
    int Init(void* uv, int port);

    int Cleanup();

    class CLiveWorker;
    struct live_session {
        virtual void AsyncSend() = 0;

        bool                  isWs;           // �Ƿ�Ϊwebsocket
        int                   error_code;     // ʧ��ʱ�Ĵ�����
        bool                  send_header;    // Ӧ���Ƿ�д��header
        CLiveWorker          *pWorker;        // worker����
        live_session();
        virtual ~live_session();
    };
};