#pragma once
#include "LiveClient.h"
#include "avtypes.h"
#include "uv.h"

namespace LiveClient
{
    class CLiveReceiver;

    class CLiveWorker : public ILiveWorker
    {
    public:
        CLiveWorker(string strCode, int rtpPort, string sdp);
        ~CLiveWorker();

        /** �ͻ������� */
        virtual bool AddHandle(ILiveHandle* h, HandleType t, int c);
        virtual bool RemoveHandle(ILiveHandle* h);
		virtual AV_BUFF GetHeader(HandleType t, int c);
        virtual string GetSDP();

        /** �ͻ���ȫ���Ͽ�����ʱ������ʵ�� */
        void Clear2Stop();
        bool m_bStop;          //< ���붨ʱ���ص�����Ϊtrue��close��ʱ���ص������ٶ���
        bool m_bOver;          //< ��ʱ����Ϊtrue���ͻ���ȫ���Ͽ�����ʱ����������

        /** ��ȡ�ͻ�����Ϣ */
        string GetClientInfo();

        /**
        * ��Դ��������Ƶ���ݣ����߳����� 
        * ���·�����rtp�������ڵ�loop�̵߳���
        * �������������������졢��������http���ڵ�loop�̵߳���
        */
        void push_flv_stream (AV_BUFF buff);
        void push_flv_stream_sub(AV_BUFF buff);
        void push_h264_stream(AV_BUFF buff);
        void push_ts_stream  (AV_BUFF buff);
        void push_fmp4_stream(AV_BUFF buff);
        void push_rtp_stream (AV_BUFF buff);
        void push_rtcp_stream(AV_BUFF buff);
        void stop();

        bool m_bFlv;
        bool m_bFlvSub;
        bool m_bMp4;
        bool m_bH264;
        bool m_bTs;
        bool m_bRtp;
		AV_BUFF               m_stFlvHead;    //flvͷ�����ݴ洢��CFlv����
		AV_BUFF               m_stMp4Head;    //mp4ͷ�����ݴ洢��CMP4����
        AV_BUFF               m_stFlvSubHead; //flv������ͷ�� ���ݴ洢��CFlv����

    private:
        string                   m_strCode;     // ����ý����
        string                   m_strSDP;      // sip���������ص�sdp
        CLiveReceiver*           m_pReceiver;   // ֱ�����ݽ��պͽ��װ��

        vector<ILiveHandle*>     m_vecLiveFlv;  // ����ʵ�� 
        CriticalSection          m_csFlv;
        vector<ILiveHandle*>     m_vecLiveFlvSub; // ����ʵ��
        CriticalSection          m_csFlvSub;
        vector<ILiveHandle*>     m_vecLiveMp4;  // ����ʵ�� 
        CriticalSection          m_csMp4;
        vector<ILiveHandle*>     m_vecLiveH264; // ����ʵ�� 
        CriticalSection          m_csH264;
        vector<ILiveHandle*>     m_vecLiveTs;   // ����ʵ�� 
        CriticalSection          m_csTs;
        vector<ILiveHandleRtp*>  m_vecLiveRtp;  // ����ʵ�� 
        CriticalSection          m_csRtp;

        int                      m_nType;          //< 0:liveֱ����1:record��ʷ��Ƶ
        int                      m_nPort;          //< rtp���ն˿�

        uv_timer_t               m_uvTimerStop;    //< http���Ŷ�ȫ���������Ӻ��ӳ����٣��Ա�ҳ��ˢ��ʱ���ٲ���
    };

    extern CLiveWorker* CreatLiveWorker(string strCode);
    extern CLiveWorker* GetLiveWorker(string strCode);
    extern bool DelLiveWorker(string strCode);
	extern string GetAllWorkerClientsInfo();
}