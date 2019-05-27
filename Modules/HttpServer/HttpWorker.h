#pragma once
#include "LiveClient.h"

namespace HttpWsServer
{
    struct pss_http_ws_live;
    enum MediaType;

    class CHttpWorker : public LiveClient::ILiveHandle
    {
    public:
        CHttpWorker(string strCode, HandleType t, int nChannel);
        ~CHttpWorker();

        /** �ͻ������� */
        bool AddConnect(pss_http_ws_live* pss);
        bool DelConnect(pss_http_ws_live* pss);

        /** ����˻�ȡ��Ƶ���� */
        AV_BUFF GetHeader();
        AV_BUFF GetVideo(uint32_t *tail);
        void NextWork(pss_http_ws_live* pss);

        virtual void push_video_stream(AV_BUFF buff);
        virtual void stop();
        virtual string get_clients_info();
    private:
        void cull_lagging_clients();

    private:
        string                m_strCode;     // ����ý����

        /**
         * lws_ring�������λ�������ֻ��һ���߳�д�룬һ���̶߳�ȡ
         * m_pRing��liveworker�е�uv_loop�߳�д�룬http�������ڵ�uv_loop�̶߳�ȡ
         */
        AV_BUFF               m_stHead;
        struct lws_ring       *m_pRing;
        pss_http_ws_live      *m_pPssList;

        int                   m_nType;          //< 0:liveֱ����1:record��ʷ��Ƶ
        LiveClient::ILiveWorker *m_pLive;       //< ֱ�����ݽ��պͽ��װ��
        HandleType            m_type;           //< ��������һ������
        int                   m_nChannel;       //< ͨ�� 0:ԭʼ����  1:С����
    };

    /** ֱ�� */
    CHttpWorker* CreatHttpWorker(string strCode, HandleType t, int nChannel);
    CHttpWorker* GetHttpWorker(string strCode, HandleType t, int nChannel);
    bool DelHttpWorker(string strCode, HandleType t, int nChannel);

    /** �㲥 */
};