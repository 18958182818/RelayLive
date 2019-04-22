#pragma once
#include "libRtsp.h"
#include "ring_buff.h"

namespace RtspServer
{
    class CRtspWorker;
    
    /** per session structure */
    typedef struct _pss_rtsp_client_ {
        struct _pss_rtsp_client_  *pss_next;
        char                path[128];              //���Ŷ������ַ
        char                clientName[50];        //���Ŷ˵�����
        char                clientIP[50];          //���Ŷ˵�ip
        char                code[50];              //�豸����
        char                strErrInfo[128];        //���ܲ���ʱ�Ĵ�����Ϣ
        ring_buff_t*        ring;             //�������ݻ�����
        uint32_t              tail;              //ringbuff�е�λ��
        bool                  culled;
        CRtspWorker*          m_pWorker;
        CClient*              rtspClient;
        bool                  playing;           //�Ƿ��ڲ���
    } pss_rtsp_client;

    extern int callback_live_rtsp(CClient *client, rtsp_method reason, void *user);

}