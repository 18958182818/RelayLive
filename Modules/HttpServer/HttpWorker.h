#pragma once
#include "LiveClient.h"

namespace HttpWsServer
{
    struct pss_http_ws_live;
    enum MediaType;

    class CHttpWorker : public LiveClient::ILiveHandle
    {
    public:
        CHttpWorker(string strCode, HandleType t, int nChannel, bool isWs);
        ~CHttpWorker();

        /** ����˻�ȡ��Ƶ���� */
        AV_BUFF GetHeader();
        AV_BUFF GetVideo(uint32_t *tail);
        void NextWork(pss_http_ws_live* pss);

        /**
         * �ײ�����H264����
         */
        virtual void push_video_stream(AV_BUFF buff);

        /**
         * �ײ�֪ͨ���Źر�(����rtp��ʱ���Է��رյ�)
         */
        virtual void stop();

        /**
         * ��װý������
         */
        void MediaCb(AV_BUFF buff);

        virtual LiveClient::ClientInfo get_clients_info();
    private:
        void cull_lagging_clients();

    public:
        string                m_strCode;        //< ����ý����
        HandleType            m_eHandleType;     //< ��������һ������
        //MediaType             m_eMediaType;
        int                   m_nChannel;       //< ͨ�� 0:ԭʼ����  1:С����
        bool                  m_bWebSocket;     //< false:http����true:websocket

        string                m_strPath;        //< ���Ŷ������ַ
        string                m_strClientName;  //< ���Ŷ˵�����
        string                m_strClientIP;    //< ���Ŷ˵�ip

    private:
        LiveClient::ILiveWorker *m_pLive;       //< ����RTP���ݲ����264����
        void                    *m_pFormat;     //< ��Ƶ��ʽ���
        struct lws_ring         *m_pRing;       //< ����ý�����ݵĻ�����
        pss_http_ws_live        *m_pPss;        //< ���ӻỰ

        int                   m_nType;          //< 0:liveֱ����1:record��ʷ��Ƶ
    };

};