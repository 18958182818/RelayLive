#pragma once
#include "LiveClient.h"
#include "uv.h"

namespace LiveClient
{
    class CLiveObj;

    class CLiveWorker : public ILiveWorker
    {
    public:
        CLiveWorker(string strCode, int rtpPort);
        ~CLiveWorker();

        /** �ͻ������� */
        virtual bool AddHandle(ILiveHandle* h, HandleType t);
        virtual bool RemoveHandle(ILiveHandle* h);
		virtual LIVE_BUFF GetHeader(HandleType t);

        /** �ͻ���ȫ���Ͽ�����ʱ������ʵ�� */
        void Clear2Stop();
        bool m_bStop;          //< ���붨ʱ���ص�����Ϊtrue��close��ʱ���ص������ٶ���
        bool m_bOver;          //< ��ʱ����Ϊtrue���ͻ���ȫ���Ͽ�����ʱ����������

        /** ��ȡ�ͻ�����Ϣ */
        static string GetClientInfo();

        /**
        * ��Դ��������Ƶ���ݣ����߳����� 
        * ���·�����rtp�������ڵ�loop�̵߳���
        * �������������������졢��������http���ڵ�loop�̵߳���
        */
        void push_flv_stream (int eType, char* pBuff, int nLen);
        void push_h264_stream(char* pBuff, int nLen);
        void push_ts_stream  (char* pBuff, int nLen);
        void push_fmp4_stream(int eType, char* pBuff, int nBuffSize);
        void push_rtp_stream (char* pBuff, int nBuffSize);
        void push_rtcp_stream(char* pBuff, int nBuffSize);
        void stop();

        bool m_bFlv;
        bool m_bMp4;
        bool m_bH264;
        bool m_bTs;
        bool m_bRtp;
		LIVE_BUFF               m_stFlvHead;    //
		LIVE_BUFF               m_stMp4Head;

    private:
        string                   m_strCode;     // ����ý����
        vector<ILiveHandle*>     m_vecLiveFlv;  // ����ʵ�� 
        CriticalSection          m_csFlv;
        vector<ILiveHandle*>     m_vecLiveMp4;  // ����ʵ�� 
        CriticalSection          m_csMp4;
        vector<ILiveHandle*>     m_vecLiveH264; // ����ʵ�� 
        CriticalSection          m_csH264;
        vector<ILiveHandle*>     m_vecLiveTs;   // ����ʵ�� 
        CriticalSection          m_csTs;
        vector<ILiveHandleRtp*>  m_vecLiveRtp;  // ����ʵ�� 
        CriticalSection          m_csRtp;

        int                      m_nType;          //< 0:liveֱ����1:record��ʷ��Ƶ
        CLiveObj*                m_pLive;          //< ֱ�����ݽ��պͽ��װ��
        int                      m_nPort;          //< rtp���ն˿�

        uv_timer_t               m_uvTimerStop;    //< http���Ŷ�ȫ���������Ӻ��ӳ����٣��Ա�ҳ��ˢ��ʱ���ٲ���
    };

    extern CLiveWorker* CreatLiveWorker(string strCode);
    extern CLiveWorker* GetLiveWorker(string strCode);
    extern bool DelLiveWorker(string strCode);
}