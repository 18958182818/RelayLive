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
        CLiveWorker(string strCode, pss_http_ws_live *pss);
        ~CLiveWorker();

        bool Play();

		/** �ͻ���ȫ���Ͽ�����ʱ������ʵ�� */
		void Clear2Stop();
        bool m_bStop;

        /** ����˻�ȡ��Ƶ���� */
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
         */
        struct lws_ring       *m_pRing;
        pss_http_ws_live      *m_pPssList;
    };

    /** ֱ�� */
    CLiveWorker* CreatLiveWorker(string strCode, pss_http_ws_live *pss);

    void InitFFmpeg();
};