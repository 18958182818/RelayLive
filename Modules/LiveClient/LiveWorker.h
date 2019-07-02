#pragma once
#include "LiveClient.h"
#include "avtypes.h"
#include "Recode.h"
#include "uv.h"

namespace LiveClient
{
    class CLiveReceiver;
    class CLiveChannel;

    class CLiveWorker : public ILiveWorker
    {
    public:
        CLiveWorker(string strCode, int rtpPort, string sdp);
        ~CLiveWorker();

        /** �ͻ������� */
        virtual bool AddHandle(ILiveHandle* h, HandleType t, int c);
        virtual bool RemoveHandle(ILiveHandle* h);
        virtual string GetSDP();

        /** �ͻ���ȫ���Ͽ�����ʱ������ʵ�� */
        void Clear2Stop();
        bool m_bStop;          //< ���붨ʱ���ص�����Ϊtrue��close��ʱ���ص������ٶ���
        bool m_bOver;          //< ��ʱ����Ϊtrue���ͻ���ȫ���Ͽ�����ʱ����������

        /** ��ȡ�ͻ�����Ϣ */
        vector<ClientInfo> GetClientInfo();

        /** ���յ�����Ƶ������ */
        void ReceiveStream(AV_BUFF buff);

        /** yuv��Ƶ���� */
        void ReceiveYUV(AV_BUFF buff);

        /** �������ݳ�ʱ����Ľ���������֪ͨ�������ӶϿ� */
        void stop();

        bool m_bRtp;

    private:
        string                   m_strCode;     // ����ý����
        string                   m_strSDP;      // sip���������ص�sdp
        CLiveReceiver           *m_pReceiver;   // ֱ�����ݽ��պͽ��

        CLiveChannel            *m_pOrigin;     // ԭʼ��ͨ��

#ifdef USE_FFMPEG
        map<int, CLiveChannel*>  m_mapChlEx;    // ��չͨ��
        CriticalSection          m_csChls;      // map����
        IDecoder                *m_pDecoder;    // h264����
#endif

        vector<ILiveHandleRtp*>  m_vecLiveRtp;  // RTPԭʼ��ת��
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