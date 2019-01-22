#pragma once

namespace HttpWsServer
{
    struct pss_http_ws_live;
    enum MediaType;

    struct LIVE_BUFF {
        char *pBuff;
        int   nLen;
    };

    class CLiveWorker
    {
    public:
        CLiveWorker(string strCode, int rtpPort);
        ~CLiveWorker();

        bool Play();

        /** �ͻ������� */
        bool AddConnect(pss_http_ws_live* pss);
        bool DelConnect(pss_http_ws_live* pss);

		/** �ͻ���ȫ���Ͽ�����ʱ������ʵ�� */
		void Clear2Stop();
        bool m_bStop;

        /** ����˻�ȡ��Ƶ���� */
        LIVE_BUFF GetFlvHeader();
        LIVE_BUFF GetFlvVideo(uint32_t *tail);
        void NextWork(pss_http_ws_live* pss);


        /**
         * ��Դ��������Ƶ���ݣ����߳����� 
         * ���¼̳���IlibLiveCb�ķ�����rtp�������ڵ�loop�̵߳���
         * �������������������졢��������http���ڵ�loop�̵߳���
         */
        void push_flv_frame(char* pBuff, int nLen);
        void stop();
    private:
        void cull_lagging_clients(MediaType type);


    private:
        string                m_strCode;     // ����ý����

        /**
         * lws_ring�������λ�������ֻ��һ���߳�д�룬һ���̶߳�ȡ
         * m_pFlvRing��m_pH264Ring��m_pMP4Ring��rtp��ȡ��loop�߳�д�룬http�������ڵ�loop�̶߳�ȡ
         */
        //flv
        LIVE_BUFF             m_stFlvHead;  //flvͷ���ݱ�����libLiveģ�飬�ⲿ����Ҫ�ͷ�
        struct lws_ring       *m_pFlvRing;
        pss_http_ws_live      *m_pFlvPssList;
    };

    /** ֱ�� */
    CLiveWorker* CreatLiveWorker(string strCode);
};