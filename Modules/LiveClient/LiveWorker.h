#pragma once
#include "LiveClient.h"
#include "avtypes.h"
#include "Recode.h"
#include "uv.h"
#include "rtp.h"

namespace LiveClient
{
    class CLiveReceiver;
    class CLiveChannel;
    struct live_event_loop_t;

    struct PlayAnswerList {
        struct PlayAnswerList *pNext;
        string strDevCode;  //�豸����
        int    nRet;        //�������󷵻�ֵ 0�ɹ� ��0ʧ��
        string strMark;     //ʧ��ʱ�������ԭ�򣬳ɹ�ʱ����sdp��Ϣ
    };

    class CLiveWorker : public ILiveWorker
    {
    public:
        CLiveWorker(string strCode, int rtpPort);
        ~CLiveWorker();

        /** �ͻ������� */
        virtual bool AddHandle(ILiveHandle* h, HandleType t, int c);
        bool AddHandleAsync(ILiveHandle* h, HandleType t, int c);
        virtual bool RemoveHandle(ILiveHandle* h);
        bool RemoveHandleAsync(ILiveHandle* h);
        virtual string GetSDP();


        /** �ͻ���ȫ���Ͽ�����ʱ������ʵ�� */
        void Clear2Stop();
        bool m_bStop;          //< ���붨ʱ���ص�����Ϊtrue��close��ʱ���ص������ٶ���
        bool m_bOver;          //< ��ʱ����Ϊtrue���ͻ���ȫ���Ͽ�����ʱ����������

        /** ��ȡ�ͻ�����Ϣ */
        vector<ClientInfo> GetClientInfo();

        /** ���յ�����Ƶ������ */
        void ReceiveStream(AV_BUFF buff);
        void ReceiveStreamAsync(AV_BUFF buff);

#ifdef EXTEND_CHANNELS
        /** yuv��Ƶ���� */
        void ReceiveYUV(AV_BUFF buff);
#endif

        /** �������ݳ�ʱ����Ľ���������֪ͨ�������ӶϿ� */
        void stop();

        /** ���Ž���ص� */
        void PlayAnswer(PlayAnswerList *pal);

        /** ����SDP */
        void ParseSdp();

    public:
        int         m_nRunNum;
        bool m_bRtp;

    private:
        string                   m_strServerIP; // rtp���Ͷ�IP
        uint32_t                 m_nServerPort; // rtp���Ͷ˿�
        uint32_t                 m_nPort;       // rtp���ն˿�
        string                   m_strCode;     // ����ý����
        bool                     m_bPlayed;     // true:���������յ�Ӧ�� false:û���յ�Ӧ��
        int                      m_nPlayRes;    // ���ŷ��صĽ�� 0�ɹ�����0ʧ��
        string                   m_strPlayInfo; // sip���������ص�sdp��ʧ��ԭ������
        RTP_STREAM_TYPE          m_stream_type; // ��Ƶ������
        
        CLiveReceiver           *m_pReceiver;   // ֱ�����ݽ��պͽ��
        CLiveChannel            *m_pOrigin;     // ԭʼ��ͨ��

#ifdef EXTEND_CHANNELS
        map<int, CLiveChannel*>  m_mapChlEx;    // ��չͨ��
        //CriticalSection          m_csChls;      // map����
        IDecoder                *m_pDecoder;    // h264����
#endif

        vector<ILiveHandleRtp*>  m_vecLiveRtp;  // RTPԭʼ��ת��
        //CriticalSection          m_csRtp;

        int                      m_nType;          //< 0:liveֱ����1:record��ʷ��Ƶ

        live_event_loop_t*       m_pEventLoop;
        //uv_timer_t               m_uvTimerStop;    //< http���Ŷ�ȫ���������Ӻ��ӳ����٣��Ա�ҳ��ˢ��ʱ���ٲ���
    };

    extern CLiveWorker* CreatLiveWorker(string strCode);
    extern CLiveWorker* GetLiveWorker(string strCode);
    extern bool DelLiveWorker(string strCode);
	extern string GetAllWorkerClientsInfo();
    extern bool AsyncPlayCB(PlayAnswerList *pal);
}