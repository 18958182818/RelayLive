#pragma once
#include "libRtsp.h"

namespace RtspServer
{
    class CRtspWorker;
    
    /** per session structure */
    struct pss_rtsp_client {
        pss_rtsp_client       *pss_next;
        string                path;              //���Ŷ������ַ
        string                clientName;        //���Ŷ˵�����
        string                clientIP;          //���Ŷ˵�ip
        string                code;              //�豸����
        string                strErrInfo;        //���ܲ���ʱ�Ĵ�����Ϣ
        struct lws_ring       *ring;             //�������ݻ�����
        uint32_t              tail;              //ringbuff�е�λ��
        bool                  culled;
        CRtspWorker*          m_pWorker;
        CClient*              rtspClient;
        bool                  playing;           //�Ƿ��ڲ���
    };

    extern int callback_live_rtsp(CClient *client, rtsp_method reason, void *user, void *in, size_t len);

}