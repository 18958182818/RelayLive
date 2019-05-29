#pragma once
#include "avtypes.h"
#include "ring_buff.h"
#include "uv.h"
#include "Netstreammaker.h"

enum NalType;
namespace LiveClient
{

    class CDecoder;
    class CEncoder;

    class CRecoder
    {
    public:
        CRecoder(void* handle=NULL);
        ~CRecoder(void);

        /**
        * ����ͨ��1����Ϣ
        */
        void SetChannel1(uint32_t nWidth, uint32_t nHeight, AV_CALLBACK cb);

        /**
        * H264�������½���롢����
        */
        int Recode(AV_BUFF buff, NalType t);

        void RecodeThread();

    private:
        int Recode2(AV_BUFF buff, NalType t);

        /**
        * ����һ����Ƶ�ϲ�����
        */
        bool EncodeVideo(char *data,int size,int bIsKeyFrame);

        /**/
        bool EncodeKeyVideo();

    private:
        void              *m_hUser;
        CDecoder          *m_decode;          // ����ԭʼ����
        CEncoder          *m_codec;           // ����������
        CNetStreamMaker   *m_pSPS;            // ����SPS���ݵĻ���
        CNetStreamMaker   *m_pPPS;            // ����PPS���ݵĻ���
        CNetStreamMaker   *m_pKeyFrame;       // ����ؼ�֡��sps��pps�п����ں���
        bool               m_bFirstKey;       // �Ѿ������һ���ؼ�֡
        bool               m_bRun;            // ִ��״̬
        bool               m_bFinish;         // ִ�����
        bool               m_bGotSPS;         // ��ǽ����Ƿ������sps
        bool               m_bGotPPS;         // ��ǽ����Ƿ������sps
        uint32_t           m_timestamp;       // ʱ���
        uint32_t           m_tick_gap;        // ��֡��ļ��
        ring_buff_t       *m_pRingH264;       // ����h264����
    };

};