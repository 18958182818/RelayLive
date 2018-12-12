#pragma once
#include "libLive.h"

namespace HttpWsServer
{
    struct pss_http_ws_live;
    enum MediaType;

    struct LIVE_BUFF {
        char *pBuff;
        int   nLen;
    };

    class CLiveWorker : public IlibLiveCb
    {
    public:
        CLiveWorker(string strCode, int rtpPort);
        ~CLiveWorker();

        /** �ͻ������� */
        bool AddConnect(pss_http_ws_live* pss);
        bool DelConnect(pss_http_ws_live* pss);

		/** �ͻ���ȫ���Ͽ�����ʱ������ʵ�� */
		void Clear2Stop();
        void SetTimeStop(bool b){m_bStop=b;}
        bool GetTimeStop(){return m_bStop;}
        /** Դ��ʱ�������Ͽ����пͻ��� */
        void Over2Stop();

        /** ��Դ��������Ƶ���ݣ����߳����� */
        void push_flv_frame(FLV_FRAG_TYPE eType, char* pBuff, int nLen);
        void push_h264_stream(char* pBuff, int nLen);
        void push_ts_stream(char* pBuff, int nLen);
        void push_mp4_stream(MP4_FRAG_TYPE eType, char* pBuff, int nBuffSize);

        /** ����˻�ȡ��Ƶ���� */
        LIVE_BUFF GetFlvHeader();
        LIVE_BUFF GetFlvVideo(uint32_t *tail);
        //------------------------------------------
        LIVE_BUFF GetH264Video(uint32_t *tail);
        //------------------------------------------
        LIVE_BUFF GetMp4Header();
        LIVE_BUFF GetMp4Video(uint32_t *tail);
        //------------------------------------------
        void NextWork(pss_http_ws_live* pss);

        /** ��ȡ�ͻ�����Ϣ */
        string GetClientInfo();
    private:
        void cull_lagging_clients(MediaType type);

        void stop();

    private:
        string                m_strCode;     // ����ý����

        //flv
        LIVE_BUFF             m_stFlvHead;  //flvͷ���ݱ�����libLiveģ�飬�ⲿ����Ҫ�ͷ�
        struct lws_ring       *m_pFlvRing;
        pss_http_ws_live      *m_pFlvPssList;

        //h264
        struct lws_ring       *m_pH264Ring;
        pss_http_ws_live      *m_pH264PssList;

        //fMP4
        LIVE_BUFF             m_stMP4Head;  //mp4ͷ���ݱ�����libLiveģ�飬�ⲿ����Ҫ�ͷ�
        struct lws_ring       *m_pMP4Ring;
        pss_http_ws_live      *m_pMP4PssList;

        int                   m_nType;          //< 0:liveֱ����1:record��ʷ��Ƶ
        IlibLive*             m_pLive;          //< ֱ�����ݽ��պͽ��װ��
        int                   m_nPort;          //< rtp���ն˿�

        uv_timer_t            m_uvTimerStop;    //< http���Ŷ�ȫ���������Ӻ��ӳ����٣��Ա�ҳ��ˢ��ʱ���ٲ���
        bool                  m_bStop;          //< ���붨ʱ���ص�����Ϊtrue��close��ʱ���ص������ٶ���
        uv_timer_t            m_uvTimerOver;    //< ���ճ�ʱ��ʱ��,һ��ʱ��û�д�Դ�յ����ݣ��Ͽ����пͻ����ӣ�����������
        bool                  m_bOver;          //< ��ʱ����Ϊtrue���ͻ���ȫ���Ͽ�����ʱ����������
    };

    /** ipc ��ʼ�� */
    void ipc_init();

    /** ֱ�� */
    CLiveWorker* CreatLiveWorker(string strCode);
    CLiveWorker* GetLiveWorker(string strCode);
    bool DelLiveWorker(string strCode);

    /** �㲥 */
};